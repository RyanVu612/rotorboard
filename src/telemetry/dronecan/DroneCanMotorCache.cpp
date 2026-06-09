#include "DroneCanMotorCache.h"

#include <algorithm>

namespace {

QString statusLabel(uint16_t statusCode)
{
    return statusCode == 0 ? QStringLiteral("OK") : QStringLiteral("FAULT");
}

double pickTemperatureCelsius(const hobbywing::StatusMsg2 &msg2, const hobbywing::StatusMsg3 &msg3, bool hasMsg2, bool hasMsg3)
{
    if (hasMsg2 && msg2.temperatureCelsius > 0) {
        return static_cast<double>(msg2.temperatureCelsius);
    }

    if (!hasMsg3) {
        return 0.0;
    }

    const uint8_t maxTemp = std::max({msg3.mosTemperatureCelsius, msg3.capTemperatureCelsius, msg3.motorTemperatureCelsius});
    return static_cast<double>(maxTemp);
}

} // namespace

void DroneCanMotorCache::setEmitCallback(EmitCallback callback)
{
    m_emitCallback = std::move(callback);
}

void DroneCanMotorCache::handleStatusMsg1(uint8_t nodeId, const hobbywing::StatusMsg1 &message, qint64 timestampMillis)
{
    MotorState &state = m_states[nodeId];
    state.msg1 = message;
    state.hasMsg1 = true;
    emitIfReady(nodeId, state, timestampMillis);
}

void DroneCanMotorCache::handleStatusMsg2(uint8_t nodeId, const hobbywing::StatusMsg2 &message, qint64 timestampMillis)
{
    MotorState &state = m_states[nodeId];
    state.msg2 = message;
    state.hasMsg2 = true;
    emitIfReady(nodeId, state, timestampMillis);
}

void DroneCanMotorCache::handleStatusMsg3(uint8_t nodeId, const hobbywing::StatusMsg3 &message, qint64 timestampMillis)
{
    MotorState &state = m_states[nodeId];
    state.msg3 = message;
    state.hasMsg3 = true;
    emitIfReady(nodeId, state, timestampMillis);
}

void DroneCanMotorCache::emitIfReady(uint8_t nodeId, MotorState &state, qint64 timestampMillis)
{
    if (!m_emitCallback || (!state.hasMsg1 && !state.hasMsg2)) {
        return;
    }

    MotorTelemetry sample;
    sample.motorId = static_cast<int>(nodeId);
    sample.rpm = state.hasMsg1 ? static_cast<double>(state.msg1.rpm) : 0.0;
    sample.pwm = state.hasMsg1 ? static_cast<double>(state.msg1.pwm) : 0.0;
    sample.voltage = state.hasMsg2 ? static_cast<double>(state.msg2.inputVoltageRaw) * 0.1 : 0.0;
    sample.current = state.hasMsg2 ? static_cast<double>(state.msg2.currentRaw) * 0.1 : 0.0;
    sample.temperatureCelsius = pickTemperatureCelsius(state.msg2, state.msg3, state.hasMsg2, state.hasMsg3);
    sample.status = state.hasMsg1 ? statusLabel(state.msg1.status) : QStringLiteral("OK");
    sample.timestampMillis = timestampMillis;

    m_emitCallback(sample);
}
