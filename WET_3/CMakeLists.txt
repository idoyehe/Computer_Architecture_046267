cmake_minimum_required(VERSION 3.6)

project(Computer_Architecture_046267)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -pedantic-errors -Werror ")

set(SOURCE_FILES dflow_main.c dflow_calc.c)
add_executable(WET_3 ${SOURCE_FILES})