cmake_minimum_required(VERSION 3.6)

project(Computer_Architecture_046267)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -pedantic-errors -Werror ")

set(SOURCE_FILES bp.c bp_api.h bp_main.c)
add_executable(WET_2 ${SOURCE_FILES})