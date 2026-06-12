cmake_minimum_required(VERSION 3.21)
project(rotorboard VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MAVLINK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mavlink_c/common)
set(LIBCANARD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libcanard)

function(rotorboard_configure_libcanard target_name)
    target_compile_definitions(${target_name} PRIVATE CANARD_64_BIT=1)
endfunction()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Network SerialPort Test)

qt_standard_project_setup(REQUIRES 6.5)

set(ROTORBOARD_MAVLINK_SOURCES
    src/telemetry/MavlinkParser.h
    src/telemetry/MavlinkParser.cpp
    src/telemetry/MavlinkTelemetrySource.h
    src/telemetry/MavlinkTelemetrySource.cpp
    src/telemetry/TelemetrySourceConfig.h
    src/telemetry/TelemetrySourceFactory.h
    src/telemetry/TelemetrySourceFactory.cpp
)

set(LIBCANARD_C_SOURCE ${LIBCANARD_DIR}/canard.c)
set_source_files_properties(${LIBCANARD_C_SOURCE} PROPERTIES LANGUAGE C)

set(ROTORBOARD_DRONECAN_SOURCES
    src/telemetry/DroneCanTelemetrySource.h
    src/telemetry/DroneCanTelemetrySource.cpp
    src/telemetry/SlcanTransport.h
    src/telemetry/SlcanTransport.cpp
    src/telemetry/dronecan/DroneCanFrameParser.h
    src/telemetry/dronecan/DroneCanFrameParser.cpp
    src/telemetry/dronecan/DroneCanMotorCache.h
    src/telemetry/dronecan/DroneCanMotorCache.cpp
    src/telemetry/dronecan/HobbywingMessages.h
    src/telemetry/dronecan/HobbywingMessages.cpp
    ${LIBCANARD_C_SOURCE}
)

qt_add_executable(rotorboard_app
    src/main.cpp
    src/app/AppController.h
    src/app/AppController.cpp
    src/model/MotorTelemetry.h
    src/model/MotorTelemetry.cpp
    src/model/WarningLevel.h
    src/telemetry/TelemetrySource.h
    src/telemetry/TelemetrySource.cpp
    src/telemetry/CsvPlaybackSource.cpp
    src/telemetry/CsvPlaybackSource.h
    src/telemetry/FakeTelemetrySource.h
    src/telemetry/FakeTelemetrySource.cpp
    src/telemetry/TelemetryManager.h
    src/telemetry/TelemetryManager.cpp
    src/logging/CsvTelemetryLogger.h
    src/logging/CsvTelemetryLogger.cpp
    src/store/MotorSampleRingBuffer.h
    src/store/MotorSampleRingBuffer.cpp
    src/store/MotorTelemetryModel.h
    src/store/MotorTelemetryModel.cpp
    src/store/DashboardLayoutModel.h
    src/store/DashboardLayoutModel.cpp
    src/warnings/MotorWarningEvaluator.h
    src/warnings/MotorWarningEvaluator.cpp
    ${ROTORBOARD_MAVLINK_SOURCES}
    ${ROTORBOARD_DRONECAN_SOURCES}
)

set_source_files_properties(qml/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)
set_source_files_properties(qml/DashboardPage.qml PROPERTIES QT_RESOURCE_ALIAS DashboardPage.qml)
set_source_files_properties(qml/DashboardGrid.qml PROPERTIES QT_RESOURCE_ALIAS DashboardGrid.qml)
set_source_files_properties(qml/WidgetDialog.qml PROPERTIES QT_RESOURCE_ALIAS WidgetDialog.qml)
set_source_files_properties(qml/MetricUtils.js PROPERTIES QT_RESOURCE_ALIAS MetricUtils.js)
set_source_files_properties(qml/StatusBadge.qml PROPERTIES QT_RESOURCE_ALIAS StatusBadge.qml)
set_source_files_properties(qml/Sparkline.qml PROPERTIES QT_RESOURCE_ALIAS Sparkline.qml)
set_source_files_properties(qml/widgets/ValueTile.qml PROPERTIES QT_RESOURCE_ALIAS widgets/ValueTile.qml)
set_source_files_properties(qml/widgets/ChartTile.qml PROPERTIES QT_RESOURCE_ALIAS widgets/ChartTile.qml)
set_source_files_properties(qml/widgets/MotorSummaryCard.qml PROPERTIES QT_RESOURCE_ALIAS widgets/MotorSummaryCard.qml)
set_source_files_properties(qml/widgets/DashboardWidgetSlot.qml PROPERTIES QT_RESOURCE_ALIAS widgets/DashboardWidgetSlot.qml)

qt_add_qml_module(rotorboard_app
    URI Rotorboard
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/DashboardPage.qml
        qml/DashboardGrid.qml
        qml/WidgetDialog.qml
        qml/MetricUtils.js
        qml/StatusBadge.qml
        qml/Sparkline.qml
        qml/widgets/ValueTile.qml
        qml/widgets/ChartTile.qml
        qml/widgets/MotorSummaryCard.qml
        qml/widgets/DashboardWidgetSlot.qml
)

target_include_directories(rotorboard_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LIBCANARD_DIR}
)
target_include_directories(rotorboard_app SYSTEM PRIVATE
    ${MAVLINK_INCLUDE_DIR}
)

target_link_libraries(rotorboard_app PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::SerialPort
)

rotorboard_configure_libcanard(rotorboard_app)

enable_testing()

qt_add_executable(rotorboard_model_tests
    tests/MotorTelemetryModelTest.cpp
    src/model/MotorTelemetry.h
    src/model/MotorTelemetry.cpp
    src/model/WarningLevel.h
    src/store/MotorSampleRingBuffer.h
    src/store/MotorSampleRingBuffer.cpp
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

qt_add_executable(rotorboard_layout_tests
    tests/DashboardLayoutModelTest.cpp
    src/store/DashboardLayoutModel.h
    src/store/DashboardLayoutModel.cpp
)

target_include_directories(rotorboard_layout_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(rotorboard_layout_tests PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME rotorboard_layout_tests COMMAND rotorboard_layout_tests)

qt_add_executable(rotorboard_ringbuffer_tests
    tests/MotorSampleRingBufferTest.cpp
    src/store/MotorSampleRingBuffer.h
    src/store/MotorSampleRingBuffer.cpp
)

target_include_directories(rotorboard_ringbuffer_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(rotorboard_ringbuffer_tests PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME rotorboard_ringbuffer_tests COMMAND rotorboard_ringbuffer_tests)

qt_add_executable(rotorboard_logger_tests
    tests/CsvTelemetryLoggerTest.cpp
    src/logging/CsvTelemetryLogger.h
    src/logging/CsvTelemetryLogger.cpp
    src/model/MotorTelemetry.h
    src/model/MotorTelemetry.cpp
    src/telemetry/TelemetrySource.h
    src/telemetry/TelemetrySource.cpp
    src/telemetry/CsvPlaybackSource.h
    src/telemetry/CsvPlaybackSource.cpp
)

target_include_directories(rotorboard_logger_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(rotorboard_logger_tests PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME rotorboard_logger_tests COMMAND rotorboard_logger_tests)

qt_add_executable(rotorboard_mavlink_tests
    tests/MavlinkTelemetrySourceTest.cpp
    src/model/MotorTelemetry.h
    src/model/MotorTelemetry.cpp
    src/telemetry/TelemetrySource.h
    src/telemetry/TelemetrySource.cpp
    src/telemetry/MavlinkParser.h
    src/telemetry/MavlinkParser.cpp
    src/telemetry/MavlinkTelemetrySource.h
    src/telemetry/MavlinkTelemetrySource.cpp
)

target_include_directories(rotorboard_mavlink_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_include_directories(rotorboard_mavlink_tests SYSTEM PRIVATE
    ${MAVLINK_INCLUDE_DIR}
)

target_link_libraries(rotorboard_mavlink_tests PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Test
)

add_test(NAME rotorboard_mavlink_tests COMMAND rotorboard_mavlink_tests)

qt_add_executable(rotorboard_dronecan_tests
    tests/DroneCanTelemetrySourceTest.cpp
    src/model/MotorTelemetry.h
    src/model/MotorTelemetry.cpp
    src/telemetry/TelemetrySource.h
    src/telemetry/TelemetrySource.cpp
    ${ROTORBOARD_DRONECAN_SOURCES}
)

target_include_directories(rotorboard_dronecan_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LIBCANARD_DIR}
)

target_link_libraries(rotorboard_dronecan_tests PRIVATE
    Qt6::Core
    Qt6::SerialPort
    Qt6::Test
)

rotorboard_configure_libcanard(rotorboard_dronecan_tests)

add_test(NAME rotorboard_dronecan_tests COMMAND rotorboard_dronecan_tests)
