#include "DroneCanTelemetrySource.h"

#include <QDateTime>
#include <QDebug>

DroneCanTelemetrySource::DroneCanTelemetrySource(const QString &serialPort, QObject *parent)
    : TelemetrySource(parent)
    , m_serialPort(serialPort)
    , m_transport(this)
{
    m_cache.setEmitCallback([this](const MotorTelemetry &telemetry) {
        emit telemetryReceived(telemetry);
    });
    m_parser.setMotorCache(&m_cache);

    connect(&m_transport, &SlcanTransport::canFrameReceived, this, &DroneCanTelemetrySource::onCanFrame);
}

void DroneCanTelemetrySource::start()
{
    if (!m_serialPort.isEmpty() && !m_transport.openPort(m_serialPort)) {
        qWarning() << "DroneCanTelemetrySource could not open" << m_serialPort;
    }
}

void DroneCanTelemetrySource::stop()
{
    m_transport.closePort();
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
