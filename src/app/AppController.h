#pragma once

#include "store/MotorTelemetryModel.h"
#include "telemetry/TelemetryManager.h"

#include <QObject>

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *telemetryModel READ telemetryModel CONSTANT)

public:
    explicit AppController(const QString &playbackPath = QString(), QObject *parent = nullptr);
    ~AppController() override;

    QObject *telemetryModel();

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void startLogging(const QString &path);
    Q_INVOKABLE void stopLogging();

private:
    MotorTelemetryModel m_telemetryModel;
    TelemetryManager m_telemetryManager;
};
