#pragma once

#include <QObject>
#include <QSerialPort>

extern "C" {
#include "canard.h"
}

class SlcanTransport : public QObject
{
    Q_OBJECT

public:
    explicit SlcanTransport(QObject *parent = nullptr);

    bool openPort(const QString &portName, qint32 baudRate = 1000000);
    void closePort();
    bool isOpen() const;

signals:
    void canFrameReceived(const CanardCANFrame &frame);

private slots:
    void onReadyRead();

private:
    bool parseLine(const QByteArray &line, CanardCANFrame *frame) const;

    QSerialPort m_serialPort;
    QByteArray m_lineBuffer;
};
