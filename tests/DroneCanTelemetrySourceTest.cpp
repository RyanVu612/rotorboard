#include "model/MotorTelemetry.h"
#include "telemetry/DroneCanTelemetrySource.h"
#include "telemetry/dronecan/HobbywingMessages.h"

#include <QSignalSpy>
#include <QTest>
#include <QVector>

extern "C" {
#include "canard.h"
}

class DroneCanTelemetrySourceTest : public QObject
{
    Q_OBJECT

private slots:
    void mergesHobbywingStatusMessagesIntoMotorTelemetry();
};

namespace {

QVector<CanardCANFrame> broadcastFrames(uint8_t sourceNodeId,
                                        uint16_t dataTypeId,
                                        uint64_t signature,
                                        const void *payload,
                                        uint16_t payloadLen)
{
    CanardInstance txInstance{};
    uint8_t memoryPool[1024]{};
    uint8_t transferId = 0;

    canardInit(&txInstance, memoryPool, sizeof(memoryPool), nullptr, nullptr, nullptr);
    canardSetLocalNodeID(&txInstance, sourceNodeId);

    const int16_t result = canardBroadcast(&txInstance,
                                           signature,
                                           dataTypeId,
                                           &transferId,
                                           hobbywing::kDefaultPriority,
                                           payload,
                                           payloadLen);
    if (result < 0) {
        return {};
    }

    QVector<CanardCANFrame> frames;
    for (const CanardCANFrame *frame = canardPeekTxQueue(&txInstance); frame != nullptr;
         frame = canardPeekTxQueue(&txInstance)) {
        frames.push_back(*frame);
        canardPopTxQueue(&txInstance);
    }
    return frames;
}

QByteArray statusMsg1Payload(uint16_t rpm, uint16_t pwm, uint16_t status)
{
    QByteArray payload(6, '\0');
    payload[0] = static_cast<char>(rpm & 0xFF);
    payload[1] = static_cast<char>((rpm >> 8) & 0xFF);
    payload[2] = static_cast<char>(pwm & 0xFF);
    payload[3] = static_cast<char>((pwm >> 8) & 0xFF);
    payload[4] = static_cast<char>(status & 0xFF);
    payload[5] = static_cast<char>((status >> 8) & 0xFF);
    return payload;
}

QByteArray statusMsg2Payload(int16_t voltageRaw, int16_t currentRaw, uint8_t temperatureCelsius)
{
    QByteArray payload(5, '\0');
    payload[0] = static_cast<char>(voltageRaw & 0xFF);
    payload[1] = static_cast<char>((voltageRaw >> 8) & 0xFF);
    payload[2] = static_cast<char>(currentRaw & 0xFF);
    payload[3] = static_cast<char>((currentRaw >> 8) & 0xFF);
    payload[4] = static_cast<char>(temperatureCelsius);
    return payload;
}

QByteArray statusMsg3Payload(uint8_t mosTemp, uint8_t capTemp, uint8_t motorTemp)
{
    QByteArray payload(3, '\0');
    payload[0] = static_cast<char>(mosTemp);
    payload[1] = static_cast<char>(capTemp);
    payload[2] = static_cast<char>(motorTemp);
    return payload;
}

void injectFrames(DroneCanTelemetrySource &source, const QVector<CanardCANFrame> &frames)
{
    for (const CanardCANFrame &frame : frames) {
        source.injectCanFrame(frame);
    }
}

} // namespace

void DroneCanTelemetrySourceTest::mergesHobbywingStatusMessagesIntoMotorTelemetry()
{
    DroneCanTelemetrySource source;
    QSignalSpy spy(&source, &DroneCanTelemetrySource::telemetryReceived);

    const QByteArray msg1 = statusMsg1Payload(3200, 1500, 0);
    const QByteArray msg2 = statusMsg2Payload(504, 185, 62);
    const QByteArray msg3 = statusMsg3Payload(70, 65, 55);

    const auto msg1Frames = broadcastFrames(5,
                                          hobbywing::kStatusMsg1Id,
                                          hobbywing::kStatusMsg1Signature,
                                          msg1.constData(),
                                          static_cast<uint16_t>(msg1.size()));
    QVERIFY(!msg1Frames.isEmpty());
    injectFrames(source, msg1Frames);
    const auto msg2Frames = broadcastFrames(5,
                                          hobbywing::kStatusMsg2Id,
                                          hobbywing::kStatusMsg2Signature,
                                          msg2.constData(),
                                          static_cast<uint16_t>(msg2.size()));
    QVERIFY(!msg2Frames.isEmpty());
    injectFrames(source, msg2Frames);

    const auto msg3Frames = broadcastFrames(5,
                                          hobbywing::kStatusMsg3Id,
                                          hobbywing::kStatusMsg3Signature,
                                          msg3.constData(),
                                          static_cast<uint16_t>(msg3.size()));
    QVERIFY(!msg3Frames.isEmpty());
    injectFrames(source, msg3Frames);

    QVERIFY(spy.count() >= 2);

    const auto lastSample = qvariant_cast<MotorTelemetry>(spy.last().at(0));
    QCOMPARE(lastSample.motorId, 5);
    QCOMPARE(lastSample.rpm, 3200.0);
    QCOMPARE(lastSample.pwm, 1500.0);
    QCOMPARE(lastSample.voltage, 50.4);
    QCOMPARE(lastSample.current, 18.5);
    QCOMPARE(lastSample.temperatureCelsius, 62.0);
    QCOMPARE(lastSample.status, QStringLiteral("OK"));
}

QTEST_MAIN(DroneCanTelemetrySourceTest)
#include "DroneCanTelemetrySourceTest.moc"
