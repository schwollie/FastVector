cmake_minimum_required(VERSION 3.26)
project(FastList)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(GTest REQUIRED)

add_executable(TEST testStorage.cpp)
target_link_libraries(TEST ${GTEST_LIBRARIES})
