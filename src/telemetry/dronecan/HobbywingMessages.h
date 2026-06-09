#pragma once

#include <cstdint>

namespace hobbywing {

constexpr uint16_t kStatusMsg1Id = 20050;
constexpr uint16_t kStatusMsg2Id = 20051;
constexpr uint16_t kStatusMsg3Id = 20052;

constexpr uint64_t kStatusMsg1Signature = 0x0813b3e2c4ad670eULL;
constexpr uint64_t kStatusMsg2Signature = 0x1675da01c3b91297ULL;
constexpr uint64_t kStatusMsg3Signature = 0x24919cd1eb34ece9ULL;

constexpr uint8_t kDefaultPriority = 20;

struct StatusMsg1 {
    uint16_t rpm = 0;
    uint16_t pwm = 0;
    uint16_t status = 0;
};

struct StatusMsg2 {
    int16_t inputVoltageRaw = 0;
    int16_t currentRaw = 0;
    uint8_t temperatureCelsius = 0;
};

struct StatusMsg3 {
    uint8_t mosTemperatureCelsius = 0;
    uint8_t capTemperatureCelsius = 0;
    uint8_t motorTemperatureCelsius = 0;
};

bool decodeStatusMsg1(const uint8_t *payload, uint16_t payloadLen, StatusMsg1 *out);
bool decodeStatusMsg2(const uint8_t *payload, uint16_t payloadLen, StatusMsg2 *out);
bool decodeStatusMsg3(const uint8_t *payload, uint16_t payloadLen, StatusMsg3 *out);

uint32_t makeExtendedCanId(uint8_t priority, uint16_t dataTypeId, uint8_t sourceNodeId);

} // namespace hobbywing
