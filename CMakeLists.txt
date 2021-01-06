cmake_minimum_required(VERSION 3.14)
project(maksima)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Og -g -Wall -Wextra -std=c++17")

add_executable(maksima
    function_maxima.h
    maxima_example.cc
    )

add_executable(ciebiera
        function_maxima.h
        tests/wyjatkowy_int.cpp
)

add_executable(damian
        function_maxima.h
        tests/test_damiana.cc
        )

add_executable(tescik
        function_maxima.h
        tests/tescik.cpp)