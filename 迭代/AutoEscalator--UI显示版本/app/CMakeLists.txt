project(app)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)  


set(PROJECT_SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/ElevatorWidget.cpp
    src/ui/LoginDialog.cpp
    src/core/Elevator.cpp
    src/core/ElevatorSystem.cpp
    src/core/EmergencyHandler.cpp
    src/core/PassengerManager.cpp
)

set(PROJECT_HEADERS
    src/ui/MainWindow.h
    src/ui/ElevatorWidget.h
    src/ui/LoginDialog.h
    src/core/Types.h
    src/core/Elevator.h
    src/core/ElevatorSystem.h
    src/core/EmergencyHandler.h
    src/core/PassengerManager.h
)

add_executable(elevator_app
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

target_include_directories(elevator_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(elevator_app PRIVATE
    Qt5::Widgets
)

