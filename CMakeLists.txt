cmake_minimum_required(VERSION 3.24)
project(DeterministicMatchingEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

include(FetchContent)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.zip)
FetchContent_MakeAvailable(googletest)

add_executable(
    tests
    tests/price_test.cpp 
    tests/order_test.cpp 
    tests/quantity_test.cpp 
    tests/slot_map_test.cpp
)
target_include_directories(tests PRIVATE include)
target_link_libraries(tests PRIVATE GTest::gtest_main)
target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
    $<$<CONFIG:Debug>:-fsanitize=address,undefined -fno-omit-frame-pointer>)
target_link_options(tests PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)

enable_testing()
include(GoogleTest)
gtest_discover_tests(tests)
