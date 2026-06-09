#pragma once

#include "SlcanTransport.h"
#include "TelemetrySource.h"
#include "dronecan/DroneCanFrameParser.h"
#include "dronecan/DroneCanMotorCache.h"

class DroneCanTelemetrySource : public TelemetrySource
{
    Q_OBJECT

public:
    explicit DroneCanTelemetrySource(const QString &serialPort = QString(),
                                     QObject *parent = nullptr);

    void start() override;
    void stop() override;

    void injectCanFrame(const CanardCANFrame &frame);

private:
    void onCanFrame(const CanardCANFrame &frame);

    QString m_serialPort;
    SlcanTransport m_transport;
    DroneCanMotorCache m_cache;
    DroneCanFrameParser m_parser;
};
