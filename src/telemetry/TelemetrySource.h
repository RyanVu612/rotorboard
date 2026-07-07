#pragma once

#include "model/BatteryTelemetry.h"
#include "model/MotorTelemetry.h"

#include <QObject>

class TelemetrySource : public QObject
{
    Q_OBJECT

public:
    // Diagnostic link states, ordered worst-to-healthy. Numeric values are part
    // of the contract with the UI (used to pick the indicator colour).
    enum LinkState {
        FailedOpen = 0,    // transport could not be opened
        NoData = 1,        // transport open, no bytes received
        NoValidFrames = 2, // bytes arriving, but none parse as valid frames
        Receiving = 3      // valid frames arriving
    };

    explicit TelemetrySource(QObject *parent = nullptr);
    virtual ~TelemetrySource() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

signals:
    void telemetryReceived(const MotorTelemetry &telemetry);
    void batteryTelemetryReceived(const BatteryTelemetry &telemetry);
    void linkStatusChanged(int state, double messageRate, const QString &endpointName);
};
