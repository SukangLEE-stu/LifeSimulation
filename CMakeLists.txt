cmake_minimum_required(VERSION 3.17)
project(LifeSimulation)

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(
        config/include
        web/include
)

add_subdirectory(utils)
add_subdirectory(screeps)
add_subdirectory(lock)
add_subdirectory(http)
add_subdirectory(web)
add_subdirectory(config)

add_executable(LifeSimulation main.cpp)
target_link_libraries(LifeSimulation config web)
