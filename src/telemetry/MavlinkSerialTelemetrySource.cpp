#include "MavlinkSerialTelemetrySource.h"

#include "MavlinkTelemetrySource.h"

#include <QDateTime>
#include <QDebug>
#include <QSerialPortInfo>

#include <cstdio>

namespace {

constexpr int kStatusTickMillis = 1000;
constexpr qint64 kDataTimeoutMillis = 2000;

// Writes a line to stdout unconditionally. Used for the serial-port diagnostics
// that must always be visible, since qInfo() is suppressed by Qt's default
// message handler. Flushed immediately so it survives an early process exit.
void logLine(const QString &line)
{
    std::fputs(line.toLocal8Bit().constData(), stdout);
    std::fputc('\n', stdout);
    std::fflush(stdout);
}

bool looksLikeUsbSerial(const QSerialPortInfo &info)
{
    const QString name = info.portName();
    const QString location = info.systemLocation();
    for (const QString &needle : {QStringLiteral("usbmodem"), QStringLiteral("usbserial"),
                                  QStringLiteral("ttyACM"), QStringLiteral("ttyUSB"),
                                  QStringLiteral("cu.usb")}) {
        if (name.contains(needle, Qt::CaseInsensitive) ||
            location.contains(needle, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

// Logs every serial port the OS reports and returns the port name to open.
// An explicit request is honoured as-is; otherwise the first plausible USB
// serial device is chosen, falling back to the first port with a USB vendor id,
// then the first port of any kind. Returns an empty string when none exist.
QString chooseSerialPort(const QString &requested)
{
    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    logLine(QStringLiteral("MavlinkSerial detected %1 serial port(s):").arg(ports.size()));
    for (const QSerialPortInfo &info : ports) {
        logLine(QStringLiteral("  - %1 (%2) %3 %4")
                    .arg(info.portName(), info.systemLocation(),
                         info.description(), info.manufacturer()));
    }

    if (!requested.isEmpty()) {
        return requested;
    }

    for (const QSerialPortInfo &info : ports) {
        if (looksLikeUsbSerial(info)) {
            return info.portName();
        }
    }
    for (const QSerialPortInfo &info : ports) {
        if (info.hasVendorIdentifier()) {
            return info.portName();
        }
    }
    if (!ports.isEmpty()) {
        return ports.first().portName();
    }
    return QString();
}

} // namespace

MavlinkSerialTelemetrySource::MavlinkSerialTelemetrySource(const QString &portName,
                                                           qint32 baudRate,
                                                           QObject *parent)
    : TelemetrySource(parent)
    , m_requestedPortName(portName)
    , m_baudRate(baudRate)
{
    connect(&m_serialPort, &QSerialPort::readyRead, this, &MavlinkSerialTelemetrySource::onReadyRead);
    m_statusTimer.setInterval(kStatusTickMillis);
    connect(&m_statusTimer, &QTimer::timeout, this, &MavlinkSerialTelemetrySource::onStatusTick);
}

void MavlinkSerialTelemetrySource::start()
{
    m_parser.reset();
    m_lastByteMillis = 0;
    m_lastFrameMillis = 0;
    m_framesSinceTick = 0;
    m_messageRate = 0.0;

    m_activePortName = chooseSerialPort(m_requestedPortName);

    if (m_activePortName.isEmpty()) {
        qWarning() << "MavlinkSerial found no serial port to open";
        m_portOpen = false;
        m_state = FailedOpen;
        emitStatus();
        return;
    }

    m_serialPort.setPortName(m_activePortName);
    m_serialPort.setBaudRate(m_baudRate);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort.open(QIODevice::ReadOnly)) {
        qWarning() << "MavlinkSerial failed to open" << m_activePortName << m_serialPort.errorString();
        m_portOpen = false;
        m_state = FailedOpen;
        emitStatus();
        return;
    }

    logLine(QStringLiteral("MavlinkSerial listening on %1 at %2 baud")
                .arg(m_activePortName).arg(m_baudRate));
    m_portOpen = true;
    m_state = NoData;
    emitStatus();
    m_statusTimer.start();
}

void MavlinkSerialTelemetrySource::stop()
{
    m_statusTimer.stop();
    if (m_serialPort.isOpen()) {
        m_serialPort.close();
    }
    m_portOpen = false;
    m_parser.reset();
}

void MavlinkSerialTelemetrySource::onReadyRead()
{
    processBytes(m_serialPort.readAll());
}

void MavlinkSerialTelemetrySource::processBytes(const QByteArray &data)
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

    if (m_portOpen) {
        updateState();
    }
}

void MavlinkSerialTelemetrySource::handleMessage(const mavlink_message_t &message)
{
    if (message.msgid != MAVLINK_MSG_ID_ESC_STATUS) {
        return;
    }

    MavlinkTelemetrySource::handleEscStatusMessage(message, [this](const MotorTelemetry &telemetry) {
        emit telemetryReceived(telemetry);
    });
}

void MavlinkSerialTelemetrySource::onStatusTick()
{
    m_messageRate = m_framesSinceTick;
    m_framesSinceTick = 0;
    updateState();
    if (m_state == Receiving) {
        emitStatus();
    }
}

void MavlinkSerialTelemetrySource::updateState()
{
    if (!m_portOpen) {
        setState(FailedOpen);
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const bool framesRecent = m_lastFrameMillis != 0 && (now - m_lastFrameMillis) < kDataTimeoutMillis;
    const bool bytesRecent = m_lastByteMillis != 0 && (now - m_lastByteMillis) < kDataTimeoutMillis;

    if (framesRecent) {
        setState(Receiving);
    } else if (bytesRecent) {
        setState(NoValidFrames);
    } else {
        setState(NoData);
    }
}

void MavlinkSerialTelemetrySource::setState(LinkState state)
{
    if (m_state == state) {
        return;
    }
    m_state = state;
    emitStatus();
}

void MavlinkSerialTelemetrySource::emitStatus()
{
    emit linkStatusChanged(static_cast<int>(m_state), m_messageRate, m_activePortName);
}
