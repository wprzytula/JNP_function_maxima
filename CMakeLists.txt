cmake_minimum_required(VERSION 3.14)
project(maksima)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_CXX_COMPILER g++)

set(CMAKE_CXX_FLAGS "-Og -g -Wall -Wextra -std=c++17")

add_executable(maksima
    function_maxima.h
    maxima_example.cc
    )
