project(app)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 添加所有头文件
file(GLOB_RECURSE HEADER_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

set(PROJECT_SOURCES
    src/main.cpp
    src/core/Elevator.cpp
    src/core/ElevatorDispatcher.cpp
    src/core/PassengerManager.cpp
    src/ui/LoginWindow.cpp
    src/ui/MainWindow.cpp
    src/ui/ElevatorWidget.cpp
    src/ui/ElevatorConfigDialog.cpp
    src/ui/StatusMonitorDialog.cpp
    src/utils/Config.cpp
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
    ${HEADER_FILES}
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

