#include "HobbywingMessages.h"

#include <cstring>

namespace hobbywing {

namespace {

bool readU16(const uint8_t *payload, uint16_t payloadLen, int offset, uint16_t *out)
{
    if (offset + 2 > payloadLen) {
        return false;
    }
    std::memcpy(out, payload + offset, 2);
    return true;
}

bool readI16(const uint8_t *payload, uint16_t payloadLen, int offset, int16_t *out)
{
    if (offset + 2 > payloadLen) {
        return false;
    }
    std::memcpy(out, payload + offset, 2);
    return true;
}

} // namespace

bool decodeStatusMsg1(const uint8_t *payload, uint16_t payloadLen, StatusMsg1 *out)
{
    if (!out || payloadLen < 6) {
        return false;
    }

    return readU16(payload, payloadLen, 0, &out->rpm) &&
           readU16(payload, payloadLen, 2, &out->pwm) &&
           readU16(payload, payloadLen, 4, &out->status);
}

bool decodeStatusMsg2(const uint8_t *payload, uint16_t payloadLen, StatusMsg2 *out)
{
    if (!out || payloadLen < 5) {
        return false;
    }

    if (!readI16(payload, payloadLen, 0, &out->inputVoltageRaw) ||
        !readI16(payload, payloadLen, 2, &out->currentRaw)) {
        return false;
    }

    out->temperatureCelsius = payload[4];
    return true;
}

bool decodeStatusMsg3(const uint8_t *payload, uint16_t payloadLen, StatusMsg3 *out)
{
    if (!out || payloadLen < 3) {
        return false;
    }

    out->mosTemperatureCelsius = payload[0];
    out->capTemperatureCelsius = payload[1];
    out->motorTemperatureCelsius = payload[2];
    return true;
}

uint32_t makeExtendedCanId(uint8_t priority, uint16_t dataTypeId, uint8_t sourceNodeId)
{
    return (static_cast<uint32_t>(priority & 0x1F) << 24) |
           (static_cast<uint32_t>(dataTypeId) << 8) |
           static_cast<uint32_t>(sourceNodeId);
}

} // namespace hobbywing
