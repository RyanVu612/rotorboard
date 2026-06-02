#include "FakeTelemetrySource.h"

#include <QDateTime>

FakeTelemetrySource::FakeTelemetrySource(QObject *parent)
    : TelemetrySource(parent)
{
    m_timer.setInterval(kIntervalMs);
    connect(&m_timer, &QTimer::timeout, this, &FakeTelemetrySource::tick);
}

void FakeTelemetrySource::start()
{
    m_timer.start();
}

void FakeTelemetrySource::stop()
{
    m_timer.stop();
}

void FakeTelemetrySource::tick()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    for (int id = 1; id < kMotorCount; id++) {
        MotorTelemetry sample;
        sample.motorId = id;
        sample.rpm = 2000.0 + QRandomGenerator::global()->bounded(4000);
        sample.voltage = 44.0 + QRandomGenerator::global()->bounded(800) / 100.0;
        sample.current = 5.0 + QRandomGenerator::global()->bounded(7500) / 100.0;
        sample.temperatureCelsius = 30.0 + QRandomGenerator::global()->bounded(4500) / 100.0;
        sample.pwm = 1000.0 + QRandomGenerator::global()->bounded(1000);
        sample.status = QStringLiteral("OK");
        sample.timestampMillis = now;

        emit telemetryReceived(sample);
    }
}