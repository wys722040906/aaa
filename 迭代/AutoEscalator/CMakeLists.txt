cmake_minimum_required(VERSION 3.10)

project(elevator_simulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 明确列出所有源文件
set(SOURCES
    src/main.cpp
    src/elevator.cpp
    src/passenger.cpp
    src/building.cpp
    src/simulator.cpp
    src/statistics.cpp
    src/utils.cpp
)

# 创建可执行文件
add_executable(elevator_simulation ${SOURCES})

# 指定头文件目录
target_include_directories(elevator_simulation PRIVATE src)


