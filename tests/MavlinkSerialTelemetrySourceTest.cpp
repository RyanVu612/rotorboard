#include "model/MotorTelemetry.h"
#include "telemetry/MavlinkSerialTelemetrySource.h"
#include "telemetry/TelemetrySource.h"

#include <QTest>
#include <QVector>

extern "C" {
#include "mavlink.h"
}

class MavlinkSerialTelemetrySourceTest : public QObject
{
    Q_OBJECT

private slots:
    void parsesEscStatusFromSerialBytes();
    void ignoresNonMavlinkBytes();
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

} // namespace

void MavlinkSerialTelemetrySourceTest::parsesEscStatusFromSerialBytes()
{
    MavlinkSerialTelemetrySource source;
    QVector<MotorTelemetry> captured;
    QObject::connect(&source, &TelemetrySource::telemetryReceived,
                     [&](const MotorTelemetry &telemetry) { captured.push_back(telemetry); });

    source.processBytes(escStatusPacket(2400, 48.0f, 20.0f, 0));

    QCOMPARE(captured.size(), 1);
    QCOMPARE(captured.at(0).motorId, 1);
    QCOMPARE(captured.at(0).rpm, 2400.0);
    QCOMPARE(captured.at(0).voltage, 48.0);
    QCOMPARE(captured.at(0).current, 20.0);
    QCOMPARE(captured.at(0).status, QStringLiteral("OK"));
}

void MavlinkSerialTelemetrySourceTest::ignoresNonMavlinkBytes()
{
    MavlinkSerialTelemetrySource source;
    QVector<MotorTelemetry> captured;
    QObject::connect(&source, &TelemetrySource::telemetryReceived,
                     [&](const MotorTelemetry &telemetry) { captured.push_back(telemetry); });

    source.processBytes(QByteArray("not a mavlink frame\x00\x01\x02", 22));

    QCOMPARE(captured.size(), 0);
}

QTEST_MAIN(MavlinkSerialTelemetrySourceTest)
#include "MavlinkSerialTelemetrySourceTest.moc"
