#include "TelemetryManager.h"

#include "logging/CsvTelemetryLogger.h"
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

    if (m_running) {
        m_source->start();
    }
}

bool TelemetryManager::isRunning() const
{
    return m_running;
}

bool TelemetryManager::startLogging(const QString &filePath)
{
    if (!m_logger) {
        m_logger = std::make_unique<CsvTelemetryLogger>();
    }

    if (!m_logger->start(filePath)) {
        return false;
    }

    m_loggingStartedAtMillis = QDateTime::currentMSecsSinceEpoch();
    return true;
}

void TelemetryManager::stopLogging()
{
    if (m_logger) {
        m_logger->stop();
    }
}

void TelemetryManager::start()
{
    m_running = true;
    if (m_source) {
        m_source->start();
    }
    m_staleTimer.start();
}

void TelemetryManager::stop()
{
    m_running = false;
    m_staleTimer.stop();
    stopLogging();

    if (m_source) {
        m_source->stop();
    }
}

void TelemetryManager::handleTelemetryReceived(const MotorTelemetry &telemetry)
{
    if (m_model) {
        m_model->updateTelemetry(telemetry);
    }

    if (m_logger && m_logger->isActive()) {
        const qint64 elapsedMillis = QDateTime::currentMSecsSinceEpoch() - m_loggingStartedAtMillis;
        m_logger->logSample(telemetry, elapsedMillis);
    }
}

void TelemetryManager::refreshStaleState()
{
    if (!m_model) {
        return;
    }

    m_model->refreshStaleState(QDateTime::currentMSecsSinceEpoch(), kStaleThresholdMillis);
}
