#include "MavlinkTelemetrySource.h"

#include <QDateTime>
#include <QDebug>
#include <limits>

namespace {
constexpr int kEscSlotsPerMessage = 4;
constexpr int kStatusTickMillis = 1000;
constexpr qint64 kDataTimeoutMillis = 2000;
} // namespace

MavlinkTelemetrySource::MavlinkTelemetrySource(IByteStreamTransport *transport, QObject *parent)
    : TelemetrySource(parent)
    , m_transport(transport)
{
    if (m_transport) {
        if (!m_transport->parent()) {
            m_transport->setParent(this);
        }
        connect(m_transport, &IByteStreamTransport::bytesReceived,
                this, &MavlinkTelemetrySource::onBytesReceived);
        connect(m_transport, &IByteStreamTransport::opened,
                this, &MavlinkTelemetrySource::onTransportOpened);
        connect(m_transport, &IByteStreamTransport::openFailed,
                this, &MavlinkTelemetrySource::onTransportOpenFailed);
    }

    m_statusTimer.setInterval(kStatusTickMillis);
    connect(&m_statusTimer, &QTimer::timeout, this, &MavlinkTelemetrySource::onStatusTick);
}

void MavlinkTelemetrySource::start()
{
    m_parser.reset();
    m_lastByteMillis = 0;
    m_lastFrameMillis = 0;
    m_framesSinceTick = 0;
    m_messageRate = 0.0;
    m_transportOpen = false;
    m_endpointName.clear();

    if (m_transport) {
        m_transport->start();
        m_statusTimer.start();
    }
}

void MavlinkTelemetrySource::stop()
{
    m_statusTimer.stop();
    if (m_transport) {
        m_transport->stop();
    }
    m_transportOpen = false;
    m_parser.reset();
}

void MavlinkTelemetrySource::processBytes(const QByteArray &data)
{
    if (data.isEmpty()) {
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    m_lastByteMillis = now;

    for (const char byte : data) {
        const auto message = m_parser.feedByte(static_cast<uint8_t>(byte));
        if (message.has_value()) {
            m_lastFrameMillis = now;
            ++m_framesSinceTick;
            handleMessage(message.value());
        }
    }

    if (m_transportOpen) {
        updateState();
    }
}

void MavlinkTelemetrySource::onBytesReceived(const QByteArray &data)
{
    processBytes(data);
}

void MavlinkTelemetrySource::onTransportOpened(const QString &endpointName)
{
    m_endpointName = endpointName;
    m_transportOpen = true;
    m_state = LinkState::NoData;
    emitStatus();
}

void MavlinkTelemetrySource::onTransportOpenFailed()
{
    m_transportOpen = false;
    setState(LinkState::FailedOpen);
}

void MavlinkTelemetrySource::onStatusTick()
{
    m_messageRate = m_framesSinceTick;
    m_framesSinceTick = 0;
    updateState();
    if (m_state == LinkState::Receiving) {
        emitStatus();
    }
}

void MavlinkTelemetrySource::updateState()
{
    if (!m_transportOpen) {
        setState(LinkState::FailedOpen);
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const bool framesRecent = m_lastFrameMillis != 0 && (now - m_lastFrameMillis) < kDataTimeoutMillis;
    const bool bytesRecent = m_lastByteMillis != 0 && (now - m_lastByteMillis) < kDataTimeoutMillis;

    if (framesRecent) {
        setState(LinkState::Receiving);
    } else if (bytesRecent) {
        setState(LinkState::NoValidFrames);
    } else {
        setState(LinkState::NoData);
    }
}

void MavlinkTelemetrySource::setState(LinkState state)
{
    if (m_state == state) {
        return;
    }
    m_state = state;
    emitStatus();
}

void MavlinkTelemetrySource::emitStatus()
{
    emit linkStatusChanged(static_cast<int>(m_state), m_messageRate, m_endpointName);
}

void MavlinkTelemetrySource::handleMessage(const mavlink_message_t &message)
{
    if (message.msgid == MAVLINK_MSG_ID_ESC_STATUS) {
        handleEscStatusMessage(message, [this](const MotorTelemetry &telemetry) {
            emit telemetryReceived(telemetry);
        });
        return;
    }

    if (message.msgid == MAVLINK_MSG_ID_BATTERY_STATUS) {
        handleBatteryStatusMessage(message, [this](const BatteryTelemetry &telemetry) {
            emit batteryTelemetryReceived(telemetry);
        });
    }
}

void MavlinkTelemetrySource::handleEscStatusMessage(
    const mavlink_message_t &message,
    const std::function<void(const MotorTelemetry &)> &emitSample)
{
    mavlink_esc_status_t escStatus{};
    mavlink_msg_esc_status_decode(&message, &escStatus);

    const qint64 timestampMillis = QDateTime::currentMSecsSinceEpoch();

    for (int slot = 0; slot < kEscSlotsPerMessage; ++slot) {
        const bool hasData = escStatus.rpm[slot] != 0 ||
                             escStatus.voltage[slot] != 0.0f ||
                             escStatus.current[slot] != 0.0f;
        if (!hasData) {
            continue;
        }

        MotorTelemetry sample;
        sample.motorId = escStatus.index + slot + 1;
        sample.rpm = static_cast<double>(escStatus.rpm[slot]);
        sample.voltage = static_cast<double>(escStatus.voltage[slot]);
        sample.current = static_cast<double>(escStatus.current[slot]);
        sample.temperatureCelsius = 0.0;
        sample.pwm = 0.0;
        sample.status = QStringLiteral("OK");
        sample.timestampMillis = timestampMillis;

        emitSample(sample);
    }
}

void MavlinkTelemetrySource::handleBatteryStatusMessage(
    const mavlink_message_t &message,
    const std::function<void(const BatteryTelemetry &)> &emitSample)
{
    mavlink_battery_status_t batteryStatus{};
    mavlink_msg_battery_status_decode(&message, &batteryStatus);

    BatteryTelemetry sample;
    sample.batteryId = batteryStatus.id;
    for (int i = 0; i < 10; ++i) {
        if (batteryStatus.voltages[i] == UINT16_MAX) {
            continue;
        }
        const double cellVoltage = batteryStatus.voltages[i] / 1000.0;
        sample.cellVoltages.push_back(cellVoltage);
        sample.voltage += cellVoltage;
    }
    sample.current = batteryStatus.current_battery == -1 ? -1.0 : batteryStatus.current_battery / 100.0;
    sample.batteryRemaining = batteryStatus.battery_remaining == -1
        ? -1.0
        : static_cast<double>(batteryStatus.battery_remaining);
    sample.temperatureCelsius = batteryStatus.temperature == INT16_MAX
        ? std::numeric_limits<double>::quiet_NaN()
        : batteryStatus.temperature / 100.0;
    sample.currentConsumedMah = static_cast<double>(batteryStatus.current_consumed);
    sample.timestampMillis = QDateTime::currentMSecsSinceEpoch();

    emitSample(sample);
}
