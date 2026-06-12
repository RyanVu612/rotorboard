#include "AppController.h"

#include "telemetry/TelemetrySourceFactory.h"

AppController::AppController(const SourceConfig &config, QObject *parent)
    : QObject(parent)
    , m_sourceLabel(sourceLabelFor(config))
    , m_telemetryModel(this)
    , m_layoutModel(this)
    , m_telemetryManager(&m_telemetryModel, this)
{
    m_telemetryManager.setSource(makeTelemetrySource(config));
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
