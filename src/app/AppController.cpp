#include "AppController.h"

#include "telemetry/TelemetrySourceFactory.h"

AppController::AppController(const SourceConfig &config, QObject *parent)
    : QObject(parent)
    , m_sourceLabel(sourceLabelFor(config))
    , m_telemetryModel(this)
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

QString AppController::sourceLabel() const
{
    return m_sourceLabel;
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
