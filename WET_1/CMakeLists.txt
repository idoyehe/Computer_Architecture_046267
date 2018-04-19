cmake_minimum_required(VERSION 3.6)

project(Computer_Architecture_046267)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 ")

set(SOURCE_FILES sim_main.c sim_mem.c sim_core.c sim_api.h)
add_executable(WET_1 ${SOURCE_FILES})