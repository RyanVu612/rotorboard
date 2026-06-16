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
    Q_PROPERTY(bool linkMonitored READ linkMonitored NOTIFY linkStatusChanged)
    Q_PROPERTY(int linkStateLevel READ linkStateLevel NOTIFY linkStatusChanged)
    Q_PROPERTY(QString linkStatusText READ linkStatusText NOTIFY linkStatusChanged)
    Q_PROPERTY(bool chartsFrozen READ chartsFrozen WRITE setChartsFrozen NOTIFY chartsFrozenChanged)
    Q_PROPERTY(QString inputMethod READ inputMethod WRITE setInputMethod NOTIFY inputSettingsChanged)
    Q_PROPERTY(QString playbackPath READ playbackPath WRITE setPlaybackPath NOTIFY inputSettingsChanged)
    Q_PROPERTY(QString mavlinkSerialPort READ mavlinkSerialPort WRITE setMavlinkSerialPort NOTIFY inputSettingsChanged)
    Q_PROPERTY(int mavlinkSerialBaud READ mavlinkSerialBaud WRITE setMavlinkSerialBaud NOTIFY inputSettingsChanged)

public:
    explicit AppController(const SourceConfig &config = SourceConfig(),
                           bool sourceConfigExplicit = false,
                           QObject *parent = nullptr);
    ~AppController() override;

    QObject *telemetryModel();
    QObject *layoutModel();
    QString sourceLabel() const;
    bool linkMonitored() const;
    int linkStateLevel() const;
    QString linkStatusText() const;

    bool chartsFrozen() const;
    void setChartsFrozen(bool frozen);
    QString inputMethod() const;
    void setInputMethod(const QString &method);
    QString playbackPath() const;
    void setPlaybackPath(const QString &path);
    QString mavlinkSerialPort() const;
    void setMavlinkSerialPort(const QString &port);
    int mavlinkSerialBaud() const;
    void setMavlinkSerialBaud(int baud);

    Q_INVOKABLE void toggleChartsFrozen();
    Q_INVOKABLE void applyTelemetrySource();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void startLogging(const QString &path);
    Q_INVOKABLE void stopLogging();

signals:
    void chartsFrozenChanged();
    void sourceLabelChanged();
    void linkStatusChanged();
    void inputSettingsChanged();

private slots:
    void handleLinkStatusChanged(int state, double messageRate, const QString &portName);

private:
    void loadSettings();
    void saveSettings() const;
    void applyConfig(const SourceConfig &config, bool persist);
    SourceConfig configFromSettings() const;
    QString inputMethodForKind(SourceKind kind) const;
    SourceKind kindForInputMethod(const QString &method) const;

    SourceConfig m_sourceConfig;
    QString m_sourceLabel;
    bool m_linkMonitored = false;
    int m_linkStateLevel = 1;
    QString m_linkStatusText;
    bool m_chartsFrozen = false;
    MotorTelemetryModel m_telemetryModel;
    DashboardLayoutModel m_layoutModel;
    TelemetryManager m_telemetryManager;
};
