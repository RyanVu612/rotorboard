#include "AppController.h"

#include "telemetry/FakeTelemetrySource.h"

#include <memory>

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_telemetryModel(this)
    , m_telemetryManager(&m_telemetryModel, this)
{
    m_telemetryManager.setSource(std::make_unique<FakeTelemetrySource>());
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
