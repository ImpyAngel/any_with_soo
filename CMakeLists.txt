cmake_minimum_required(VERSION 3.6)
project(any_with_soo)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES main.cpp my_any.h)
add_executable(any_with_soo ${SOURCE_FILES})