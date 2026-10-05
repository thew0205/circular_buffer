cmake_minimum_required(VERSION 3.12)

project(circular_buffer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
    PRIVATE
    src/circular_buffer.cpp

)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CB_TESTS)
    add_subdirectory(tests)
endif(CB_TESTS)