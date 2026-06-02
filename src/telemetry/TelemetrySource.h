#pragma once

#include "model/MotorTelemetry.h"

#include <QObject>

class TelemetrySource : public QObject
{
    Q_OBJECT

public:
    explicit TelemetrySource(QObject *parent = nullptr);
    virtual ~TelemetrySource() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

signals:
    void telemetryReceived(const MotorTelemetry &telemetry);
};