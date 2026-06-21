#include "SerialByteStreamTransport.h"

#include <QDebug>
#include <QSerialPortInfo>

#include <cstdio>

namespace {

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

SerialByteStreamTransport::SerialByteStreamTransport(const QString &portName, qint32 baudRate, QObject *parent)
    : IByteStreamTransport(parent)
    , m_requestedPortName(portName)
    , m_baudRate(baudRate)
{
    connect(&m_serialPort, &QSerialPort::readyRead, this, &SerialByteStreamTransport::onReadyRead);
}

void SerialByteStreamTransport::start()
{
    const QString portName = chooseSerialPort(m_requestedPortName);

    if (portName.isEmpty()) {
        qWarning() << "SerialByteStreamTransport found no serial port to open";
        emit openFailed();
        return;
    }

    m_serialPort.setPortName(portName);
    m_serialPort.setBaudRate(m_baudRate);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort.open(QIODevice::ReadOnly)) {
        qWarning() << "SerialByteStreamTransport failed to open" << portName << m_serialPort.errorString();
        emit openFailed();
        return;
    }

    logLine(QStringLiteral("MavlinkSerial listening on %1 at %2 baud").arg(portName).arg(m_baudRate));
    emit opened(portName);
}

void SerialByteStreamTransport::stop()
{
    if (m_serialPort.isOpen()) {
        m_serialPort.close();
    }
}

void SerialByteStreamTransport::onReadyRead()
{
    const QByteArray data = m_serialPort.readAll();
    if (!data.isEmpty()) {
        emit bytesReceived(data);
    }
}
