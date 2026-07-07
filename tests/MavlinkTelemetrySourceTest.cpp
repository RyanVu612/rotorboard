#include "model/BatteryTelemetry.h"
#include "model/MotorTelemetry.h"
#include "telemetry/MavlinkParser.h"
#include "telemetry/MavlinkTelemetrySource.h"

#include <QTest>
#include <QtGlobal>
#include <QVector>

extern "C" {
#include "mavlink.h"
}

class MavlinkTelemetrySourceTest : public QObject
{
    Q_OBJECT

private slots:
    void parsesEscStatusIntoMotorTelemetry();
    void parsesBatteryStatusIntoBatteryTelemetry();
    void parsesBatteryStatusUnknownSentinels();
};

namespace {

QByteArray escStatusPacket(int32_t rpm, float voltage, float current, uint8_t escIndex = 0)
{
    mavlink_message_t message{};
    int32_t rpmValues[4] = {rpm, 0, 0, 0};
    float voltageValues[4] = {voltage, 0.0f, 0.0f, 0.0f};
    float currentValues[4] = {current, 0.0f, 0.0f, 0.0f};

    mavlink_msg_esc_status_pack(1, 1, &message, escIndex, 0, rpmValues, voltageValues, currentValues);

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN] = {};
    const uint16_t length = mavlink_msg_to_send_buffer(buffer, &message);
    return QByteArray(reinterpret_cast<const char *>(buffer), static_cast<int>(length));
}

QByteArray batteryStatusPacket(int16_t temperature,
                               const uint16_t voltages[10],
                               int16_t currentBattery,
                               int32_t currentConsumed,
                               int8_t batteryRemaining)
{
    mavlink_message_t message{};
    uint16_t voltagesExt[4] = {0, 0, 0, 0};
    mavlink_msg_battery_status_pack(1,
                                    1,
                                    &message,
                                    0,
                                    0,
                                    0,
                                    temperature,
                                    voltages,
                                    currentBattery,
                                    currentConsumed,
                                    0,
                                    batteryRemaining,
                                    0,
                                    0,
                                    voltagesExt,
                                    0,
                                    0);

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN] = {};
    const uint16_t length = mavlink_msg_to_send_buffer(buffer, &message);
    return QByteArray(reinterpret_cast<const char *>(buffer), static_cast<int>(length));
}

} // namespace

void MavlinkTelemetrySourceTest::parsesEscStatusIntoMotorTelemetry()
{
    MavlinkParser parser;
    QVector<MotorTelemetry> captured;
    const QByteArray packet = escStatusPacket(2400, 48.0f, 20.0f, 0);

    for (const char byte : packet) {
        const auto message = parser.feedByte(static_cast<uint8_t>(byte));
        if (message.has_value()) {
            MavlinkTelemetrySource::handleEscStatusMessage(message.value(), [&](const MotorTelemetry &telemetry) {
                captured.push_back(telemetry);
            });
        }
    }

    QCOMPARE(captured.size(), 1);
    QCOMPARE(captured.at(0).motorId, 1);
    QCOMPARE(captured.at(0).rpm, 2400.0);
    QCOMPARE(captured.at(0).voltage, 48.0);
    QCOMPARE(captured.at(0).current, 20.0);
    QCOMPARE(captured.at(0).status, QStringLiteral("OK"));
}

void MavlinkTelemetrySourceTest::parsesBatteryStatusIntoBatteryTelemetry()
{
    MavlinkParser parser;
    QVector<BatteryTelemetry> captured;
    uint16_t voltages[10] = {4100, 4150, 4120, 4180, 4090, 4110, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX};
    const QByteArray packet = batteryStatusPacket(4500, voltages, 1500, 1200, 75);

    for (const char byte : packet) {
        const auto message = parser.feedByte(static_cast<uint8_t>(byte));
        if (message.has_value()) {
            MavlinkTelemetrySource::handleBatteryStatusMessage(message.value(), [&](const BatteryTelemetry &telemetry) {
                captured.push_back(telemetry);
            });
        }
    }

    QCOMPARE(captured.size(), 1);
    const BatteryTelemetry telemetry = captured.at(0);
    QCOMPARE(telemetry.batteryId, 0);
    QCOMPARE(telemetry.cellVoltages.size(), 6);
    QCOMPARE(telemetry.cellVoltages.at(0), 4.1);
    QCOMPARE(telemetry.cellVoltages.at(1), 4.15);
    QCOMPARE(telemetry.cellVoltages.at(2), 4.12);
    QCOMPARE(telemetry.cellVoltages.at(3), 4.18);
    QCOMPARE(telemetry.cellVoltages.at(4), 4.09);
    QCOMPARE(telemetry.cellVoltages.at(5), 4.11);
    QCOMPARE(telemetry.voltage, 24.75);
    QCOMPARE(telemetry.current, 15.0);
    QCOMPARE(telemetry.batteryRemaining, 75.0);
    QCOMPARE(telemetry.temperatureCelsius, 45.0);
    QCOMPARE(telemetry.currentConsumedMah, 1200.0);
}

void MavlinkTelemetrySourceTest::parsesBatteryStatusUnknownSentinels()
{
    MavlinkParser parser;
    QVector<BatteryTelemetry> captured;
    uint16_t voltages[10] = {22000, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX,
                             UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX};
    const QByteArray packet = batteryStatusPacket(INT16_MAX, voltages, -1, 1200, -1);

    for (const char byte : packet) {
        const auto message = parser.feedByte(static_cast<uint8_t>(byte));
        if (message.has_value()) {
            MavlinkTelemetrySource::handleBatteryStatusMessage(message.value(), [&](const BatteryTelemetry &telemetry) {
                captured.push_back(telemetry);
            });
        }
    }

    QCOMPARE(captured.size(), 1);
    QCOMPARE(captured.at(0).current, -1.0);
    QCOMPARE(captured.at(0).batteryRemaining, -1.0);
    QVERIFY(qIsNaN(captured.at(0).temperatureCelsius));
}

QTEST_MAIN(MavlinkTelemetrySourceTest)
#include "MavlinkTelemetrySourceTest.moc"
