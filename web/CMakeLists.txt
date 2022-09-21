cmake_minimum_required(VERSION 3.17)

project(web)

set(CMAKE_CXX_STANDARD 17)

set(LIBRARY_OUTPUT_PATH  ${PROJECT_BINARY_DIR}/lib)

include_directories(
        include
        ../utils/include
        ../http/include
        ../lock/include
)

add_library(web web.cpp WebServer.cpp)

target_link_libraries(web utils http)