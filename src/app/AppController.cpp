#include "AppController.h"

#include "telemetry/TelemetrySource.h"
#include "telemetry/TelemetrySourceFactory.h"

#include <QSettings>

namespace {

constexpr auto kInputMethodFake = "fake";
constexpr auto kInputMethodPlayback = "playback";
constexpr auto kInputMethodPixhawk = "pixhawk";
constexpr auto kInputMethodMavlinkTcp = "mavlink_tcp";

} // namespace

AppController::AppController(const SourceConfig &config, bool sourceConfigExplicit, QObject *parent)
    : QObject(parent)
    , m_sourceConfig(config)
    , m_sourceLabel(sourceLabelFor(config))
    , m_linkMonitored(config.kind == SourceKind::MavlinkSerial || config.kind == SourceKind::MavlinkTcp)
    , m_telemetryModel(this)
    , m_layoutModel(this)
    , m_telemetryManager(&m_telemetryModel, this)
{
    if (!sourceConfigExplicit) {
        loadSettings();
    }
    applyConfig(m_sourceConfig, false);
}

AppController::~AppController()
{
    stop();
}

QObject *AppController::telemetryModel()
{
    return &m_telemetryModel;
}

QObject *AppController::layoutModel()
{
    return &m_layoutModel;
}

QString AppController::sourceLabel() const
{
    return m_sourceLabel;
}

bool AppController::linkMonitored() const
{
    return m_linkMonitored;
}

int AppController::linkStateLevel() const
{
    return m_linkStateLevel;
}

QString AppController::linkStatusText() const
{
    return m_linkStatusText;
}

QString AppController::inputMethod() const
{
    return inputMethodForKind(m_sourceConfig.kind);
}

void AppController::setInputMethod(const QString &method)
{
    const SourceKind kind = kindForInputMethod(method);
    if (m_sourceConfig.kind == kind) {
        return;
    }
    m_sourceConfig.kind = kind;
    emit inputSettingsChanged();
}

QString AppController::playbackPath() const
{
    return m_sourceConfig.playbackPath;
}

void AppController::setPlaybackPath(const QString &path)
{
    if (m_sourceConfig.playbackPath == path) {
        return;
    }
    m_sourceConfig.playbackPath = path;
    emit inputSettingsChanged();
}

QString AppController::mavlinkSerialPort() const
{
    return m_sourceConfig.mavlinkSerialPort;
}

void AppController::setMavlinkSerialPort(const QString &port)
{
    if (m_sourceConfig.mavlinkSerialPort == port) {
        return;
    }
    m_sourceConfig.mavlinkSerialPort = port;
    emit inputSettingsChanged();
}

int AppController::mavlinkSerialBaud() const
{
    return m_sourceConfig.mavlinkSerialBaud;
}

void AppController::setMavlinkSerialBaud(int baud)
{
    if (m_sourceConfig.mavlinkSerialBaud == baud) {
        return;
    }
    m_sourceConfig.mavlinkSerialBaud = baud;
    emit inputSettingsChanged();
}

QString AppController::mavlinkTcpHost() const
{
    return m_sourceConfig.mavlinkTcpHost;
}

void AppController::setMavlinkTcpHost(const QString &host)
{
    if (m_sourceConfig.mavlinkTcpHost == host) {
        return;
    }
    m_sourceConfig.mavlinkTcpHost = host;
    emit inputSettingsChanged();
}

int AppController::mavlinkTcpPort() const
{
    return m_sourceConfig.mavlinkTcpPort;
}

void AppController::setMavlinkTcpPort(int port)
{
    const quint16 clamped = static_cast<quint16>(qBound(1, port, 65535));
    if (m_sourceConfig.mavlinkTcpPort == clamped) {
        return;
    }
    m_sourceConfig.mavlinkTcpPort = clamped;
    emit inputSettingsChanged();
}

void AppController::handleLinkStatusChanged(int state, double messageRate, const QString &portName)
{
    m_linkStateLevel = state;

    const QString resolvedPort = portName.isEmpty() ? QStringLiteral("(no device)") : portName;
    const QString label = (m_sourceConfig.kind == SourceKind::MavlinkTcp)
        ? QStringLiteral("MAVLink TCP %1").arg(resolvedPort)
        : QStringLiteral("MAVLink serial %1").arg(resolvedPort);
    if (label != m_sourceLabel) {
        m_sourceLabel = label;
        emit sourceLabelChanged();
    }

    switch (state) {
    case TelemetrySource::FailedOpen:
        m_linkStatusText = QStringLiteral("port open failed");
        break;
    case TelemetrySource::NoData:
        m_linkStatusText = QStringLiteral("waiting for data");
        break;
    case TelemetrySource::NoValidFrames:
        m_linkStatusText = QStringLiteral("no valid MAVLink");
        break;
    case TelemetrySource::Receiving:
        m_linkStatusText = QStringLiteral("%1 msg/s").arg(qRound(messageRate));
        break;
    default:
        m_linkStatusText.clear();
        break;
    }

    emit linkStatusChanged();
}

bool AppController::chartsFrozen() const
{
    return m_chartsFrozen;
}

void AppController::setChartsFrozen(bool frozen)
{
    if (m_chartsFrozen == frozen) {
        return;
    }
    m_chartsFrozen = frozen;
    emit chartsFrozenChanged();
}

void AppController::toggleChartsFrozen()
{
    setChartsFrozen(!m_chartsFrozen);
}

void AppController::applyTelemetrySource()
{
    applyConfig(configFromSettings(), true);
}

void AppController::start()
{
    m_telemetryManager.start();
}

void AppController::stop()
{
    m_telemetryManager.stop();
}

void AppController::startLogging(const QString &path)
{
    m_telemetryManager.startLogging(path);
}

void AppController::stopLogging()
{
    m_telemetryManager.stopLogging();
}

void AppController::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("TelemetryInput"));

    const QString method = settings.value(QStringLiteral("inputMethod"), inputMethod()).toString();
    m_sourceConfig.kind = kindForInputMethod(method);
    m_sourceConfig.playbackPath = settings.value(QStringLiteral("playbackPath"), m_sourceConfig.playbackPath).toString();
    m_sourceConfig.mavlinkSerialPort =
        settings.value(QStringLiteral("mavlinkSerialPort"), m_sourceConfig.mavlinkSerialPort).toString();
    m_sourceConfig.mavlinkSerialBaud =
        settings.value(QStringLiteral("mavlinkSerialBaud"), m_sourceConfig.mavlinkSerialBaud).toInt();
    m_sourceConfig.mavlinkTcpHost =
        settings.value(QStringLiteral("mavlinkTcpHost"), m_sourceConfig.mavlinkTcpHost).toString();
    m_sourceConfig.mavlinkTcpPort =
        static_cast<quint16>(settings.value(QStringLiteral("mavlinkTcpPort"), m_sourceConfig.mavlinkTcpPort).toInt());

    settings.endGroup();
}

void AppController::saveSettings() const
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("TelemetryInput"));

    settings.setValue(QStringLiteral("inputMethod"), inputMethod());
    settings.setValue(QStringLiteral("playbackPath"), m_sourceConfig.playbackPath);
    settings.setValue(QStringLiteral("mavlinkSerialPort"), m_sourceConfig.mavlinkSerialPort);
    settings.setValue(QStringLiteral("mavlinkSerialBaud"), m_sourceConfig.mavlinkSerialBaud);
    settings.setValue(QStringLiteral("mavlinkTcpHost"), m_sourceConfig.mavlinkTcpHost);
    settings.setValue(QStringLiteral("mavlinkTcpPort"), static_cast<int>(m_sourceConfig.mavlinkTcpPort));

    settings.endGroup();
}

void AppController::applyConfig(const SourceConfig &config, bool persist)
{
    m_sourceConfig = config;
    m_sourceLabel = sourceLabelFor(m_sourceConfig);
    m_linkMonitored = m_sourceConfig.kind == SourceKind::MavlinkSerial
                   || m_sourceConfig.kind == SourceKind::MavlinkTcp;
    m_linkStateLevel = m_linkMonitored ? 1 : 3;
    m_linkStatusText = m_linkMonitored ? QStringLiteral("waiting for data") : QString();

    std::unique_ptr<TelemetrySource> source = makeTelemetrySource(m_sourceConfig);
    if (m_linkMonitored) {
        connect(source.get(), &TelemetrySource::linkStatusChanged,
                this, &AppController::handleLinkStatusChanged);
    }

    m_telemetryModel.clear();
    m_telemetryManager.setSource(std::move(source));

    if (persist) {
        saveSettings();
    }

    emit inputSettingsChanged();
    emit sourceLabelChanged();
    emit linkStatusChanged();
}

SourceConfig AppController::configFromSettings() const
{
    SourceConfig config = m_sourceConfig;
    config.kind = kindForInputMethod(inputMethod());
    return config;
}

QString AppController::inputMethodForKind(SourceKind kind) const
{
    switch (kind) {
    case SourceKind::Playback:
        return QString::fromLatin1(kInputMethodPlayback);
    case SourceKind::Mavlink:
    case SourceKind::MavlinkSerial:
        return QString::fromLatin1(kInputMethodPixhawk);
    case SourceKind::MavlinkTcp:
        return QString::fromLatin1(kInputMethodMavlinkTcp);
    case SourceKind::Fake:
    default:
        return QString::fromLatin1(kInputMethodFake);
    }
}

SourceKind AppController::kindForInputMethod(const QString &method) const
{
    if (method == QLatin1String(kInputMethodPlayback)) {
        return SourceKind::Playback;
    }
    if (method == QLatin1String(kInputMethodPixhawk)) {
        return SourceKind::MavlinkSerial;
    }
    if (method == QLatin1String(kInputMethodMavlinkTcp)) {
        return SourceKind::MavlinkTcp;
    }
    return SourceKind::Fake;
}
