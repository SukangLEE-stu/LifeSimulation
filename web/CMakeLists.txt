cmake_minimum_required(VERSION 3.17)

project(web)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(web web.cpp)

target_link_libraries(utils)