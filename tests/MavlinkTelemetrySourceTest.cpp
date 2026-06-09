#include "model/MotorTelemetry.h"
#include "telemetry/MavlinkParser.h"
#include "telemetry/MavlinkTelemetrySource.h"

#include <QTest>
#include <QVector>

extern "C" {
#include "mavlink.h"
}

class MavlinkTelemetrySourceTest : public QObject
{
    Q_OBJECT

private slots:
    void parsesEscStatusIntoMotorTelemetry();
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

QTEST_MAIN(MavlinkTelemetrySourceTest)
#include "MavlinkTelemetrySourceTest.moc"
