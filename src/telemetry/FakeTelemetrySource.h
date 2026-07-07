#pragma once

#include "TelemetrySource.h"

#include <QTimer>
#include <QRandomGenerator>

class FakeTelemetrySource : public TelemetrySource
{
    Q_OBJECT

public:
    explicit FakeTelemetrySource(QObject *parent = nullptr);

    void start() override;
    void stop() override;

private:
    void tick();

    QTimer m_timer;
    static constexpr int kMotorCount = 4;
    static constexpr int kIntervalMs = 100;
    double m_batteryRemaining = 100.0;
};
