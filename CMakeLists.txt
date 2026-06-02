cmake_minimum_required(VERSION 3.21)
project(rotorboard VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(rotorboard_app
    src/main.cpp
    src/model/MotorTelemetry.cpp
    src/telemetry/TelemetrySource.cpp
    src/telemetry/FakeTelemetrySource.cpp
)

target_include_directories(rotorboard_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(rotorboard_app PRIVATE Qt6::Core)