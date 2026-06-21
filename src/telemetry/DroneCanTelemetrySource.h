#pragma once

#include "ICanTransport.h"
#include "TelemetrySource.h"
#include "dronecan/DroneCanFrameParser.h"
#include "dronecan/DroneCanMotorCache.h"

class DroneCanTelemetrySource : public TelemetrySource
{
    Q_OBJECT

public:
    // transport may be nullptr (use injectCanFrame directly for testing).
    // When transport has no parent, this source takes ownership via setParent.
    explicit DroneCanTelemetrySource(ICanTransport *transport = nullptr,
                                     QObject *parent = nullptr);

    void start() override;
    void stop() override;

    void injectCanFrame(const CanardCANFrame &frame);

private:
    void onCanFrame(const CanardCANFrame &frame);

    ICanTransport *m_transport;
    DroneCanMotorCache m_cache;
    DroneCanFrameParser m_parser;
};
