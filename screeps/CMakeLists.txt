cmake_minimum_required(VERSION 3.17)

project(screeps)

set(CMAKE_CXX_STANDARD 17)

set(LIBRARY_OUTPUT_PATH  ${PROJECT_BINARY_DIR}/lib)

include_directories(
        include
)

add_library(screeps SHARED creep.cpp)
