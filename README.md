# Circular Buffer (Test-Driven Development)

A fixed-capacity circular (ring) buffer of bytes, written in C++17 using
**test-driven development (TDD)** with the [CppUTest](https://cpputest.github.io/)
framework.

The buffer is a header-only class template: capacity is set at compile time,
there is no dynamic allocation, and every operation runs in O(1). That makes it
a good fit for embedded work, such as queuing bytes between an interrupt
handler and the main loop.

---

## Table of Contents

- [Project Layout](#project-layout)
- [Requirements](#requirements)
- [Building and Running the Tests](#building-and-running-the-tests)
- [Using the Buffer](#using-the-buffer)
- [API Reference](#api-reference)
- [How It Works](#how-it-works)
- [The TDD Approach](#the-tdd-approach)
- [Test Suite](#test-suite)
- [Adding a New Test](#adding-a-new-test)

---

## Project Layout

```
circular_buffer/
├── CMakeLists.txt              # Builds the circular_buffer library; adds tests when CB_TESTS=ON
├── include/
│   └── circular_buffer.hpp     # The CircularBuffer<N> template (all the logic lives here)
├── src/
│   └── circular_buffer.cpp     # Translation unit for the static library
└── tests/
    ├── CMakeLists.txt          # Fetches CppUTest v4.0 and builds the test executable
    ├── circular_buffer_test.cpp# The unit tests
    ├── test_main.cpp           # CppUTest runner entry point
    └── cpputest/               # Local copy of CppUTest (not used by the build; see below)
```

> The build gets CppUTest with CMake `FetchContent` (tag `v4.0`), so the
> `tests/cpputest/` folder is not used. To build offline, uncomment the
> `add_subdirectory(cpputest ...)` line in `tests/CMakeLists.txt` and remove the
> `FetchContent` block.

---

## Requirements

| Tool              | Version            |
|-------------------|--------------------|
| CMake             | 3.12 or newer      |
| C++ compiler      | C++17 (GCC, Clang) |
| Git               | Needed by `FetchContent` to download CppUTest |
| Internet access   | First configure only, to fetch CppUTest |

---

## Building and Running the Tests

Tests build only when the `CB_TESTS` option is on.

```bash
# Configure (first run downloads CppUTest)
cmake -S . -B build -DCB_TESTS=ON

# Build
cmake --build build -j

# Run the tests
./build/tests/circular_buffer_test
```

Expected output:

```
......
OK (6 tests, 6 ran, 53 checks, 0 ignored, 0 filtered out, 0 ms)
```

Each `.` is a passing test. The runner uses colored output (`-c`). A failure
prints the file, line and expected vs. actual values.

To build only the library, without tests:

```bash
cmake -S . -B build
cmake --build build
```

---

## Using the Buffer

```cpp
#include "circular_buffer.hpp"

CircularBuffer<8> rx;                 // holds up to 8 bytes

rx.push(0x41);                        // CIRCULAR_BUFFER_OK
rx.push(0x42);

int byte;
if (rx.pop(byte) == CIRCULAR_BUFFER_OK)
{
    // byte == 0x41  (first in, first out)
}

if (rx.isFull())  { /* drop data or apply back-pressure */ }
if (rx.isEmpty()) { /* nothing to read */ }
```

If you omit the template argument, `CircularBuffer<>` uses a capacity of **10**.

To link it from another CMake project:

```cmake
add_subdirectory(path/to/circular_buffer)
target_link_libraries(my_app PRIVATE circular_buffer)
```

---

## API Reference

### `template <size_t bufferCapacity = 10> class CircularBuffer`

| Method | Returns | Description |
|---|---|---|
| `CircularBuffer()` | — | Creates an empty buffer with its storage set to zero. |
| `push(uint8_t value)` | `CircularBufferError` | Adds a byte at the tail. Returns `CIRCULAR_BUFFER_FULL` and changes nothing if the buffer is full. |
| `pop(int &value)` | `CircularBufferError` | Removes the oldest byte and writes it to `value`. Returns `CIRCULAR_BUFFER_EMPTY` and leaves `value` unchanged if the buffer is empty. |
| `size()` | `size_t` | Number of bytes stored now. |
| `capacity()` | `size_t` | Maximum number of bytes (`bufferCapacity`). |
| `isEmpty()` | `bool` | `true` when `size() == 0`. |
| `isFull()` | `bool` | `true` when `size() == capacity()`. |
| `hasSpace()` | `bool` | Same as `!isFull()`. |

### `enum CircularBufferError`

| Value | Meaning |
|---|---|
| `CIRCULAR_BUFFER_OK` | The operation succeeded. |
| `CIRCULAR_BUFFER_FULL` | `push` was rejected because the buffer is full. |
| `CIRCULAR_BUFFER_EMPTY` | `pop` was rejected because the buffer is empty. |

---

## How It Works

The buffer stores bytes in a fixed array and tracks two indices:

- **`head_`**: where the next `pop` reads from (the oldest element).
- **`tail_`**: where the next `push` writes to.

Both indices move forward with `(index + 1) % bufferCapacity`, so they wrap
back to `0` at the end of the array.

```
capacity = 5, after push(1), push(2), push(3), pop()

 index:   0    1    2    3    4
        [ - | 2 | 3 | - | - ]
              ^         ^
            head_     tail_        size() = 2
```

When `head_ == tail_`, the buffer is either completely empty or completely
full. A separate **`isFull_`** flag tells the two cases apart:

| `head_ == tail_` | `isFull_` | State |
|---|---|---|
| yes | `false` | empty |
| yes | `true`  | full  |
| no  | `false` | partly filled |

`push` sets `isFull_` when the tail catches up with the head. `pop` always
clears it. Because of the flag, all `capacity` slots can be used; no slot has
to be left empty to mark "full".

---

## The TDD Approach

The code was written in short **Red → Green → Refactor** cycles:

1. **Red**: write a small test for the next behavior. Run it and watch it fail
   (or fail to compile).
2. **Green**: write the least code that makes the test pass.
3. **Refactor**: clean up the code and tests while all tests stay green.

The tests grow from the simplest case to the hardest, and each one adds one new
requirement:

| Step | Behavior added | Test |
|---|---|---|
| 1 | A new buffer is empty, not full, has the right capacity, and `pop` fails | `TestEmptyOnInit` |
| 2 | `pop` on a new buffer reports empty | `NoPopAfterCreate` |
| 3 | A pushed value can be popped back | `PopAfterPush` |
| 4 | `size()` grows with each push; `push` fails once full | `TestPushUntilFull` |
| 5 | After a pop frees a slot, a push succeeds again (wrap-around) | `TestCanPushAfterPopAfterFull` |
| 6 | A full fill-then-drain cycle keeps FIFO order and the right size | `TestPushPop` |

Ordering the tests this way kept each code change small. It also forced the
full/empty problem described above to be solved before wrap-around was tested.

---

## Test Suite

All tests are in [`tests/circular_buffer_test.cpp`](tests/circular_buffer_test.cpp)
under the `CircularBufferTestGroup` test group, with a buffer capacity of 5.

| Test | What it checks |
|---|---|
| `TestEmptyOnInit` | `isEmpty()` is true, `isFull()` is false, `capacity()` is 5, `size()` is 0, and `pop` returns `CIRCULAR_BUFFER_EMPTY`. |
| `NoPopAfterCreate` | `pop` on a new buffer returns `CIRCULAR_BUFFER_EMPTY`. |
| `PopAfterPush` | Pushing `10` and then popping returns `10`. |
| `TestPushUntilFull` | `size()` goes up by one with each push; the buffer is full after 5 pushes; a 6th push returns `CIRCULAR_BUFFER_FULL`. |
| `TestCanPushAfterPopAfterFull` | When the buffer is full, one pop frees space for one more push. |
| `TestPushPop` | Fill completely, then drain completely. Values come out in insertion order, `size()` goes down by one with each pop, and the buffer ends empty. |

CppUTest's memory-leak detector is on by default, so every test also checks
that no heap memory is leaked.

---

## Adding a New Test

Follow the same cycle: write the failing test first.

```cpp
TEST(CircularBufferTestGroup, TestWrapAroundKeepsOrder)
{
    CircularBuffer<3> cb{};
    int v;

    cb.push(1); cb.push(2); cb.push(3);   // full
    cb.pop(v);                            // frees index 0
    cb.push(4);                           // tail wraps to index 0

    cb.pop(v); LONGS_EQUAL(2, v);
    cb.pop(v); LONGS_EQUAL(3, v);
    cb.pop(v); LONGS_EQUAL(4, v);
    CHECK_TRUE(cb.isEmpty());
}
```

Then rebuild and run:

```bash
cmake --build build -j && ./build/tests/circular_buffer_test
```

Commonly used CppUTest assertions:

| Macro | Use |
|---|---|
| `CHECK_TRUE(x)` / `CHECK_FALSE(x)` | Boolean conditions |
| `LONGS_EQUAL(expected, actual)` | Integer equality |
| `ENUMS_EQUAL_INT(expected, actual)` | Enum equality (prints the numeric value on failure) |
| `FAIL("message")` | Fail the test on purpose, for example as a placeholder in the Red step |

To run part of the suite, pass runner flags such as `-g <group>`,
`-n <test name>` or `-v`. Note that
[`tests/test_main.cpp`](tests/test_main.cpp) currently passes a fixed argument
list (`-c`) to the runner. It must forward `argc`/`argv` before command-line
flags will work.
