cmake_minimum_required(VERSION 3.26)
project(FastList)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3 -gdwarf-4")

find_package(GTest REQUIRED)

add_executable(TEST testStorage.cpp
        main.cpp)
target_link_libraries(TEST ${GTEST_LIBRARIES})
