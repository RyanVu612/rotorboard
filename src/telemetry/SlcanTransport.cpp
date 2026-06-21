#include "SlcanTransport.h"

#include <QDebug>

namespace {

bool parseHexNibble(char ch, uint8_t *out)
{
    if (ch >= '0' && ch <= '9') {
        *out = static_cast<uint8_t>(ch - '0');
        return true;
    }
    if (ch >= 'A' && ch <= 'F') {
        *out = static_cast<uint8_t>(10 + ch - 'A');
        return true;
    }
    if (ch >= 'a' && ch <= 'f') {
        *out = static_cast<uint8_t>(10 + ch - 'a');
        return true;
    }
    return false;
}

bool parseHexByte(const char *text, uint8_t *out)
{
    uint8_t high = 0;
    uint8_t low = 0;
    if (!parseHexNibble(text[0], &high) || !parseHexNibble(text[1], &low)) {
        return false;
    }
    *out = static_cast<uint8_t>((high << 4) | low);
    return true;
}

} // namespace

SlcanTransport::SlcanTransport(const QString &portName, qint32 baudRate, QObject *parent)
    : ICanTransport(parent)
    , m_portName(portName)
    , m_baudRate(baudRate)
{
    connect(&m_serialPort, &QSerialPort::readyRead, this, &SlcanTransport::onReadyRead);
}

void SlcanTransport::start()
{
    stop();

    m_serialPort.setPortName(m_portName);
    m_serialPort.setBaudRate(m_baudRate);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort.open(QIODevice::ReadWrite)) {
        qWarning() << "SlcanTransport failed to open" << m_portName << m_serialPort.errorString();
        emit openFailed();
        return;
    }

    m_serialPort.write("C\r");
    m_serialPort.write("S6\r");
    m_serialPort.write("O\r");
    m_serialPort.flush();
    emit opened(m_portName);
}

void SlcanTransport::stop()
{
    if (m_serialPort.isOpen()) {
        m_serialPort.write("C\r");
        m_serialPort.flush();
        m_serialPort.close();
    }
    m_lineBuffer.clear();
}

void SlcanTransport::onReadyRead()
{
    m_lineBuffer.append(m_serialPort.readAll());

    int newlineIndex = -1;
    while ((newlineIndex = m_lineBuffer.indexOf('\r')) >= 0) {
        const QByteArray line = m_lineBuffer.left(newlineIndex);
        m_lineBuffer.remove(0, newlineIndex + 1);

        CanardCANFrame frame{};
        if (parseLine(line, &frame)) {
            emit canFrameReceived(frame);
        }
    }
}

bool SlcanTransport::parseLine(const QByteArray &line, CanardCANFrame *frame) const
{
    if (!frame || line.size() < 5) {
        return false;
    }

    const char type = line.at(0);
    if (type != 'T' && type != 't') {
        return false;
    }

    const bool extended = (type == 'T');
    const int idHexLen = extended ? 8 : 3;
    if (line.size() < 1 + idHexLen + 1) {
        return false;
    }

    uint32_t canId = 0;
    for (int i = 0; i < idHexLen; ++i) {
        uint8_t nibble = 0;
        if (!parseHexNibble(line.at(1 + i), &nibble)) {
            return false;
        }
        canId = (canId << 4) | nibble;
    }

    uint8_t dlc = 0;
    if (!parseHexByte(line.constData() + 1 + idHexLen, &dlc)) {
        return false;
    }

    if (dlc > CANARD_CAN_FRAME_MAX_DATA_LEN) {
        return false;
    }

    const int dataOffset = 1 + idHexLen + 2;
    if (line.size() < dataOffset + dlc * 2) {
        return false;
    }

    frame->id = canId;
    frame->data_len = dlc;
    for (int i = 0; i < dlc; ++i) {
        if (!parseHexByte(line.constData() + dataOffset + i * 2, &frame->data[i])) {
            return false;
        }
    }

    return true;
}
