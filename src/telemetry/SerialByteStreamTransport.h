#pragma once

#include "IByteStreamTransport.h"

#include <QSerialPort>

class SerialByteStreamTransport : public IByteStreamTransport
{
    Q_OBJECT

public:
    explicit SerialByteStreamTransport(const QString &portName = QString(),
                                       qint32 baudRate = 115200,
                                       QObject *parent = nullptr);

    void start() override;
    void stop() override;

private slots:
    void onReadyRead();

private:
    QString m_requestedPortName;
    qint32 m_baudRate;
    QSerialPort m_serialPort;
};
