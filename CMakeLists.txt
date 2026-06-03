cmake_minimum_required(VERSION 3.21)
project(rotorboard VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Test)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(rotorboard_app
    src/main.cpp
    src/app/AppController.h
    src/app/AppController.cpp
    src/model/MotorTelemetry.h
    src/model/MotorTelemetry.cpp
    src/model/WarningLevel.h
    src/telemetry/TelemetrySource.h
    src/telemetry/TelemetrySource.cpp
    src/telemetry/FakeTelemetrySource.h
    src/telemetry/FakeTelemetrySource.cpp
    src/telemetry/TelemetryManager.h
    src/telemetry/TelemetryManager.cpp
    src/store/MotorTelemetryModel.h
    src/store/MotorTelemetryModel.cpp
    src/warnings/MotorWarningEvaluator.h
    src/warnings/MotorWarningEvaluator.cpp
)

set_source_files_properties(qml/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)
set_source_files_properties(qml/DashboardPage.qml PROPERTIES QT_RESOURCE_ALIAS DashboardPage.qml)
set_source_files_properties(qml/MotorGrid.qml PROPERTIES QT_RESOURCE_ALIAS MotorGrid.qml)
set_source_files_properties(qml/MotorCard.qml PROPERTIES QT_RESOURCE_ALIAS MotorCard.qml)
set_source_files_properties(qml/StatusBadge.qml PROPERTIES QT_RESOURCE_ALIAS StatusBadge.qml)

qt_add_qml_module(rotorboard_app
    URI Rotorboard
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/DashboardPage.qml
        qml/MotorGrid.qml
        qml/MotorCard.qml
        qml/StatusBadge.qml
)

target_include_directories(rotorboard_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(rotorboard_app PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

enable_testing()

qt_add_executable(rotorboard_model_tests
    tests/MotorTelemetryModelTest.cpp
    src/model/MotorTelemetry.h
    src/model/MotorTelemetry.cpp
    src/model/WarningLevel.h
    src/store/MotorTelemetryModel.h
    src/store/MotorTelemetryModel.cpp
    src/warnings/MotorWarningEvaluator.h
    src/warnings/MotorWarningEvaluator.cpp
)

target_include_directories(rotorboard_model_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(rotorboard_model_tests PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME rotorboard_model_tests COMMAND rotorboard_model_tests)
