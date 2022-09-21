cmake_minimum_required(VERSION 3.17)

project(config)

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(
        include
        ../utils/include
        ../http/include
)

add_library(config config.cpp include/config.h)
