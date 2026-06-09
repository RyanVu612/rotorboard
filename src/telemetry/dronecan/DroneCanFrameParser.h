#pragma once

#include "DroneCanMotorCache.h"

#include <cstdint>
#include <functional>

extern "C" {
#include "canard.h"
}

class DroneCanFrameParser
{
public:
    DroneCanFrameParser();

    void setMotorCache(DroneCanMotorCache *cache);
    void handleCanFrame(const CanardCANFrame &frame, qint64 timestampMillis);

private:
    static bool shouldAcceptTransfer(const CanardInstance *ins,
                                     uint64_t *outDataTypeSignature,
                                     uint16_t dataTypeId,
                                     CanardTransferType transferType,
                                     uint8_t sourceNodeId);

    static void onTransferReception(CanardInstance *ins, CanardRxTransfer *transfer);

    CanardInstance m_instance{};
    uint8_t m_memoryPool[2048]{};
    DroneCanMotorCache *m_cache = nullptr;
};
