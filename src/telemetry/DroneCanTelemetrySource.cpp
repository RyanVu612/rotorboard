#include "DroneCanTelemetrySource.h"

#include <QDateTime>
#include <QDebug>

DroneCanTelemetrySource::DroneCanTelemetrySource(ICanTransport *transport, QObject *parent)
    : TelemetrySource(parent)
    , m_transport(transport)
{
    m_cache.setEmitCallback([this](const MotorTelemetry &telemetry) {
        emit telemetryReceived(telemetry);
    });
    m_parser.setMotorCache(&m_cache);

    if (m_transport) {
        if (!m_transport->parent()) {
            m_transport->setParent(this);
        }
        connect(m_transport, &ICanTransport::canFrameReceived,
                this, &DroneCanTelemetrySource::onCanFrame);
    }
}

void DroneCanTelemetrySource::start()
{
    if (m_transport) {
        m_transport->start();
    }
}

void DroneCanTelemetrySource::stop()
{
    if (m_transport) {
        m_transport->stop();
    }
}

void DroneCanTelemetrySource::injectCanFrame(const CanardCANFrame &frame)
{
    onCanFrame(frame);
}

void DroneCanTelemetrySource::onCanFrame(const CanardCANFrame &frame)
{
    const qint64 timestampMillis = QDateTime::currentMSecsSinceEpoch();
    m_parser.handleCanFrame(frame, timestampMillis);
}
