#include "AppController.h"

#include "telemetry/CsvPlaybackSource.h"
#include "telemetry/FakeTelemetrySource.h"

#include <memory>

AppController::AppController(const QString &playbackPath, QObject *parent)
    : QObject(parent)
    , m_telemetryModel(this)
    , m_telemetryManager(&m_telemetryModel, this)
{
    if (playbackPath.isEmpty()) {
        m_telemetryManager.setSource(std::make_unique<FakeTelemetrySource>());
    } else {
        m_telemetryManager.setSource(std::make_unique<CsvPlaybackSource>(playbackPath));
    }
}

AppController::~AppController()
{
    stop();
}

QObject *AppController::telemetryModel()
{
    return &m_telemetryModel;
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
