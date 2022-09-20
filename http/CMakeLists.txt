cmake_minimum_required(VERSION 3.17)

project(http)

set(CMAKE_CXX_STANDARD 17)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(
        include
        ../utils
)

add_library(http SHARED HttpConnection.cpp)