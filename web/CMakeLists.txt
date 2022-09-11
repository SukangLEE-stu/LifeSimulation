cmake_minimum_required(VERSION 3.17)

project(web)

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(
        include
        ../utils/include
)

add_executable(web web.cpp)

target_link_libraries(web utils)