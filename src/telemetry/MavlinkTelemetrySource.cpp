#include "MavlinkTelemetrySource.h"

#include <QDateTime>
#include <QDebug>
#include <QNetworkDatagram>

namespace {
constexpr int kEscSlotsPerMessage = 4;
}

MavlinkTelemetrySource::MavlinkTelemetrySource(const QString &host, quint16 port, QObject *parent)
    : TelemetrySource(parent)
    , m_host(host)
    , m_port(port)
{
    connect(&m_socket, &QUdpSocket::readyRead, this, &MavlinkTelemetrySource::onReadyRead);
}

void MavlinkTelemetrySource::start()
{
    m_parser.reset();

    QHostAddress bindAddress;
    if (m_host.isEmpty() || m_host == QLatin1String("0.0.0.0")) {
        bindAddress = QHostAddress::AnyIPv4;
    } else {
        bindAddress.setAddress(m_host);
    }

    if (!m_socket.bind(bindAddress, m_port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qWarning() << "MavlinkTelemetrySource failed to bind" << bindAddress << m_port << m_socket.errorString();
    }
}

void MavlinkTelemetrySource::stop()
{
    m_socket.close();
    m_parser.reset();
}

void MavlinkTelemetrySource::onReadyRead()
{
    while (m_socket.hasPendingDatagrams()) {
        const QNetworkDatagram datagram = m_socket.receiveDatagram();
        const QByteArray payload = datagram.data();

        for (const char byte : payload) {
            const auto message = m_parser.feedByte(static_cast<uint8_t>(byte));
            if (message.has_value()) {
                handleMessage(message.value());
            }
        }
    }
}

void MavlinkTelemetrySource::handleMessage(const mavlink_message_t &message)
{
    if (message.msgid != MAVLINK_MSG_ID_ESC_STATUS) {
        return;
    }

    handleEscStatusMessage(message, [this](const MotorTelemetry &telemetry) {
        emit telemetryReceived(telemetry);
    });
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
