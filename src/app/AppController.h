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
    Q_PROPERTY(QString sourceLabel READ sourceLabel NOTIFY sourceLabelChanged)
    Q_PROPERTY(bool linkMonitored READ linkMonitored CONSTANT)
    Q_PROPERTY(int linkStateLevel READ linkStateLevel NOTIFY linkStatusChanged)
    Q_PROPERTY(QString linkStatusText READ linkStatusText NOTIFY linkStatusChanged)
    Q_PROPERTY(bool chartsFrozen READ chartsFrozen WRITE setChartsFrozen NOTIFY chartsFrozenChanged)

public:
    explicit AppController(const SourceConfig &config = SourceConfig(), QObject *parent = nullptr);
    ~AppController() override;

    QObject *telemetryModel();
    QObject *layoutModel();
    QString sourceLabel() const;
    bool linkMonitored() const;
    int linkStateLevel() const;
    QString linkStatusText() const;

    bool chartsFrozen() const;
    void setChartsFrozen(bool frozen);

    Q_INVOKABLE void toggleChartsFrozen();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void startLogging(const QString &path);
    Q_INVOKABLE void stopLogging();

signals:
    void chartsFrozenChanged();
    void sourceLabelChanged();
    void linkStatusChanged();

private slots:
    void handleLinkStatusChanged(int state, double messageRate, const QString &portName);

private:
    QString m_sourceLabel;
    bool m_linkMonitored = false;
    int m_linkStateLevel = 1;
    QString m_linkStatusText;
    bool m_chartsFrozen = false;
    MotorTelemetryModel m_telemetryModel;
    DashboardLayoutModel m_layoutModel;
    TelemetryManager m_telemetryManager;
};
