#pragma once

#include "telemetry/TelemetrySource.h"
#include "logging/CsvTelemetryLogger.h"

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
    bool isRunning() const;

    bool startLogging(const QString &filePath);
    void stopLogging();

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
    std::unique_ptr<CsvTelemetryLogger> m_logger;
    qint64 m_loggingStartedAtMillis = 0;
    QTimer m_staleTimer;
    bool m_running = false;
};
