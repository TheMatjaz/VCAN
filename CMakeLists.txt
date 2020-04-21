cmake_minimum_required(VERSION 3.15)
project(vcan C)

set(CMAKE_C_STANDARD 11)

add_library(vcan vcan.c vcan.h)
