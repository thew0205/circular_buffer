cmake_minimum_required(VERSION 3.12)

project(circular_buffer_test LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

set(TESTS ON CACHE BOOL "Build tests" FORCE)
# add_subdirectory(cpputest ${CMAKE_CURRENT_BINARY_DIR}/cpputest)
include(FetchContent)

FetchContent_Declare(
  CppUTest
  GIT_REPOSITORY https://github.com/cpputest/cpputest.git
  GIT_TAG v4.0
)

FetchContent_MakeAvailable(CppUTest)

add_executable(${PROJECT_NAME} 
                    circular_buffer_test.cpp
                    test_main.cpp
                    )

target_link_libraries(${PROJECT_NAME} PRIVATE 
        circular_buffer 
        CppUTest 
        CppUTestExt)
