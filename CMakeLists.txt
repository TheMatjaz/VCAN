cmake_minimum_required(VERSION 3.15)
project(vcan C)

set(CMAKE_C_STANDARD 11)

include_directories(inc/)
add_library(vcan src/vcan.c)
include_directories(tst/)
add_executable(testvcan src/vcan.c inc/vcan.h tst/atto.c tst/test.c)
