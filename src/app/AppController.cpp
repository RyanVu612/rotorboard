#include "AppController.h"

#include "telemetry/MavlinkSerialTelemetrySource.h"
#include "telemetry/TelemetrySourceFactory.h"

AppController::AppController(const SourceConfig &config, QObject *parent)
    : QObject(parent)
    , m_sourceLabel(sourceLabelFor(config))
    , m_linkMonitored(config.kind == SourceKind::MavlinkSerial)
    , m_telemetryModel(this)
    , m_layoutModel(this)
    , m_telemetryManager(&m_telemetryModel, this)
{
    std::unique_ptr<TelemetrySource> source = makeTelemetrySource(config);
    if (m_linkMonitored) {
        if (auto *serialSource = dynamic_cast<MavlinkSerialTelemetrySource *>(source.get())) {
            connect(serialSource, &MavlinkSerialTelemetrySource::linkStatusChanged,
                    this, &AppController::handleLinkStatusChanged);
        }
    }
    m_telemetryManager.setSource(std::move(source));
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

void AppController::handleLinkStatusChanged(int state, double messageRate, const QString &portName)
{
    m_linkStateLevel = state;

    const QString resolvedPort = portName.isEmpty() ? QStringLiteral("(no device)") : portName;
    const QString label = QStringLiteral("MAVLink serial %1").arg(resolvedPort);
    if (label != m_sourceLabel) {
        m_sourceLabel = label;
        emit sourceLabelChanged();
    }

    switch (state) {
    case MavlinkSerialTelemetrySource::FailedOpen:
        m_linkStatusText = QStringLiteral("port open failed");
        break;
    case MavlinkSerialTelemetrySource::NoData:
        m_linkStatusText = QStringLiteral("waiting for data");
        break;
    case MavlinkSerialTelemetrySource::NoValidFrames:
        m_linkStatusText = QStringLiteral("no valid MAVLink");
        break;
    case MavlinkSerialTelemetrySource::Receiving:
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
