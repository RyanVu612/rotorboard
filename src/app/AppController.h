#pragma once

#include "store/DashboardLayoutModel.h"
#include "store/MotorTelemetryModel.h"
#include "telemetry/TelemetryManager.h"
#include "telemetry/TelemetrySourceConfig.h"

#include <QObject>

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *telemetryModel READ telemetryModel CONSTANT)
    Q_PROPERTY(QObject *layoutModel READ layoutModel CONSTANT)
    Q_PROPERTY(QString sourceLabel READ sourceLabel CONSTANT)
    Q_PROPERTY(bool chartsFrozen READ chartsFrozen WRITE setChartsFrozen NOTIFY chartsFrozenChanged)
    Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged)

public:
    explicit AppController(const SourceConfig &config = SourceConfig(), QObject *parent = nullptr);
    ~AppController() override;

    QObject *telemetryModel();
    QObject *layoutModel();
    QString sourceLabel() const;

    bool chartsFrozen() const;
    void setChartsFrozen(bool frozen);

    bool editMode() const;
    void setEditMode(bool enabled);

    Q_INVOKABLE void toggleChartsFrozen();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void startLogging(const QString &path);
    Q_INVOKABLE void stopLogging();

signals:
    void chartsFrozenChanged();
    void editModeChanged();

private:
    QString m_sourceLabel;
    bool m_chartsFrozen = false;
    bool m_editMode = false;
    MotorTelemetryModel m_telemetryModel;
    DashboardLayoutModel m_layoutModel;
    TelemetryManager m_telemetryManager;
};
