cmake_minimum_required(VERSION 3.14)
project(elevator_system VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt5包
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

add_subdirectory(app)


