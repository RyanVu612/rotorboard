#pragma once

#include "telemetry/TelemetrySource.h"

#include <QObject>
#include <QTimer>

#include <memory>

class MotorTelemetryModel;

class TelemetryManager : public QObject
{
    Q_OBJECT

public:
    explicit TelemetryManager(MotorTelemetryModel *model, QObject *parent = nullptr);

    void setSource(std::unique_ptr<TelemetrySource> source);

public slots:
    void start();
    void stop();

private slots:
    void handleTelemetryReceived(const MotorTelemetry &telemetry);
    void refreshStaleState();

private:
    static constexpr qint64 kStaleThresholdMillis = 2000;
    static constexpr int kStaleRefreshIntervalMillis = 250;

    MotorTelemetryModel *m_model = nullptr;
    std::unique_ptr<TelemetrySource> m_source;
    QTimer m_staleTimer;
};
