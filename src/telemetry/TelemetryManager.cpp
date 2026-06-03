#include "TelemetryManager.h"

#include "store/MotorTelemetryModel.h"

#include <QDateTime>

TelemetryManager::TelemetryManager(MotorTelemetryModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
{
    m_staleTimer.setInterval(kStaleRefreshIntervalMillis);
    connect(&m_staleTimer, &QTimer::timeout, this, &TelemetryManager::refreshStaleState);
}

void TelemetryManager::setSource(std::unique_ptr<TelemetrySource> source)
{
    if (m_source) {
        m_source->stop();
        disconnect(m_source.get(), nullptr, this, nullptr);
    }

    m_source = std::move(source);

    if (!m_source) {
        return;
    }

    connect(m_source.get(), &TelemetrySource::telemetryReceived,
            this, &TelemetryManager::handleTelemetryReceived);
}

void TelemetryManager::start()
{
    if (m_source) {
        m_source->start();
    }
    m_staleTimer.start();
}

void TelemetryManager::stop()
{
    m_staleTimer.stop();

    if (m_source) {
        m_source->stop();
    }
}

void TelemetryManager::handleTelemetryReceived(const MotorTelemetry &telemetry)
{
    if (m_model) {
        m_model->updateTelemetry(telemetry);
    }
}

void TelemetryManager::refreshStaleState()
{
    if (!m_model) {
        return;
    }

    m_model->refreshStaleState(QDateTime::currentMSecsSinceEpoch(), kStaleThresholdMillis);
}
