#pragma once

#include "store/MotorTelemetryModel.h"
#include "telemetry/TelemetryManager.h"
#include "telemetry/TelemetrySourceConfig.h"

#include <QObject>

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *telemetryModel READ telemetryModel CONSTANT)
    Q_PROPERTY(QString sourceLabel READ sourceLabel CONSTANT)

public:
    explicit AppController(const SourceConfig &config = SourceConfig(), QObject *parent = nullptr);
    ~AppController() override;

    QObject *telemetryModel();
    QString sourceLabel() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void startLogging(const QString &path);
    Q_INVOKABLE void stopLogging();

private:
    QString m_sourceLabel;
    MotorTelemetryModel m_telemetryModel;
    TelemetryManager m_telemetryManager;
};
