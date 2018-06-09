cmake_minimum_required(VERSION 3.6)
project(Operation_Systems_234123)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

set(SOURCE_FILES cacheEntry.cpp cacheEntry.h cacheLevel.cpp cacheLevel.h main.cpp)

add_executable(WET_4 ${SOURCE_FILES})