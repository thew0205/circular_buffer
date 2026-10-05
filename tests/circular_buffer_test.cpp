#include "circular_buffer.hpp"

#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"

// Test when buffersize is 0

TEST_GROUP(CircularBufferTestGroup){

    void setup() override{}

    void teardown() override{}};

TEST(CircularBufferTestGroup, TestEmptyOnInit)
{
    constexpr static size_t bufferCapacity = 5;
    CircularBuffer<bufferCapacity> circularBuffer{};
    CHECK_TRUE(circularBuffer.isEmpty());
    CHECK_FALSE(circularBuffer.isFull());
    LONGS_EQUAL(bufferCapacity, circularBuffer.capacity());
    LONGS_EQUAL(0, circularBuffer.size());
    int temp;
    ENUMS_EQUAL_INT(CIRCULAR_BUFFER_EMPTY, circularBuffer.pop(temp));
}

TEST(CircularBufferTestGroup, NoPopAfterCreate)
{
    constexpr static size_t bufferCapacity = 5;
    CircularBuffer<bufferCapacity> circularBuffer{};
    ENUMS_EQUAL_INT(CIRCULAR_BUFFER_EMPTY, circularBuffer.pop(*(int *)nullptr));
}

TEST(CircularBufferTestGroup, PopAfterPush)
{
    constexpr static size_t bufferCapacity = 5;
    CircularBuffer<bufferCapacity> circularBuffer{};
    ENUMS_EQUAL_INT(CIRCULAR_BUFFER_OK, circularBuffer.push(10));
    int value = -1;
    ENUMS_EQUAL_INT(CIRCULAR_BUFFER_OK, circularBuffer.pop(value));
    LONGS_EQUAL(10, value);
}

TEST(CircularBufferTestGroup, TestPushUntilFull)
{
    constexpr static size_t bufferCapacity = 5;
    CircularBuffer<bufferCapacity> circularBuffer{};
    for (int i = 0; i < bufferCapacity; i++)
    {
        ENUMS_EQUAL_INT(CIRCULAR_BUFFER_OK, circularBuffer.push(i));
        LONGS_EQUAL(i + 1, circularBuffer.size());
    }
    CHECK_TRUE(circularBuffer.isFull());
    ENUMS_EQUAL_INT(CIRCULAR_BUFFER_FULL, circularBuffer.push(100));
}


TEST(CircularBufferTestGroup, TestCanPushAfterPopAfterFull)
{
    constexpr static size_t bufferCapacity = 5;
    CircularBuffer<bufferCapacity> circularBuffer{};
    for (int i = 0; i < bufferCapacity; i++)
    {
        circularBuffer.push(i);
    }

    CHECK_TRUE(circularBuffer.isFull());
    int value = -1;
    ENUMS_EQUAL_INT(CIRCULAR_BUFFER_OK, circularBuffer.pop(value));
    ENUMS_EQUAL_INT(CIRCULAR_BUFFER_OK, circularBuffer.push(0xA5));
}
TEST(CircularBufferTestGroup, TestPushPop)
{
    constexpr static size_t bufferCapacity = 5;
    CircularBuffer<bufferCapacity> circularBuffer{};
    for (int i = 0; i < bufferCapacity; i++)
    {
        ENUMS_EQUAL_INT(CIRCULAR_BUFFER_OK, circularBuffer.push(i));
        LONGS_EQUAL(i + 1, circularBuffer.size());
    }
    CHECK_TRUE(circularBuffer.isFull());
    ENUMS_EQUAL_INT(CIRCULAR_BUFFER_FULL, circularBuffer.push(100));

    for (int i = 0; i < bufferCapacity; i++)
    {
        int value = -1;
        ENUMS_EQUAL_INT(CIRCULAR_BUFFER_OK, circularBuffer.pop(value));
        LONGS_EQUAL(bufferCapacity - i - 1, circularBuffer.size());
        LONGS_EQUAL(i, value);
    }
    CHECK_TRUE(circularBuffer.isEmpty());
    int temp;
    ENUMS_EQUAL_INT(CIRCULAR_BUFFER_EMPTY, circularBuffer.pop(temp));
}