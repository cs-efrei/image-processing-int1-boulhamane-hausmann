cmake_minimum_required(VERSION 3.31)
project(untitled)

set(CMAKE_C_STANDARD 99)

add_executable(untitled main.c bmp8.c bmp24.c equalize8.c equalize24.c)
