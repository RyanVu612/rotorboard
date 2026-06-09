#pragma once

#include "HobbywingMessages.h"

#include "model/MotorTelemetry.h"

#include <QHash>
#include <functional>

class DroneCanMotorCache
{
public:
    using EmitCallback = std::function<void(const MotorTelemetry &)>;

    void setEmitCallback(EmitCallback callback);
    void handleStatusMsg1(uint8_t nodeId, const hobbywing::StatusMsg1 &message, qint64 timestampMillis);
    void handleStatusMsg2(uint8_t nodeId, const hobbywing::StatusMsg2 &message, qint64 timestampMillis);
    void handleStatusMsg3(uint8_t nodeId, const hobbywing::StatusMsg3 &message, qint64 timestampMillis);

private:
    struct MotorState {
        hobbywing::StatusMsg1 msg1{};
        hobbywing::StatusMsg2 msg2{};
        hobbywing::StatusMsg3 msg3{};
        bool hasMsg1 = false;
        bool hasMsg2 = false;
        bool hasMsg3 = false;
    };

    void emitIfReady(uint8_t nodeId, MotorState &state, qint64 timestampMillis);

    QHash<uint8_t, MotorState> m_states;
    EmitCallback m_emitCallback;
};
