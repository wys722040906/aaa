cmake_minimum_required(VERSION 3.10)
project(elevator_system)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt包
find_package(Qt5 COMPONENTS 
    Core
    Gui
    Widgets
    Charts
    REQUIRED
)

set(PROJECT_SOURCES
    src/main.cpp
    src/login/loginwindow.cpp
    src/login/loginwindow.h
    src/mainwindow/mainwindow.cpp
    src/mainwindow/mainwindow.h
    src/elevator/elevatorwidget.cpp
    src/elevator/elevatorwidget.h
    src/floor/floorwidget.cpp
    src/floor/floorwidget.h
    src/controller/elevatorcontroller.cpp
    src/controller/elevatorcontroller.h
    src/settings/settingsdialog.cpp
    src/settings/settingsdialog.h
    src/statistics/statisticsdialog.cpp
    src/statistics/statisticsdialog.h
    src/common/types.h
    resources/resources.qrc
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Charts
)

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

