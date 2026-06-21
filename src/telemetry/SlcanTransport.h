#pragma once

#include "ICanTransport.h"

#include <QSerialPort>

class SlcanTransport : public ICanTransport
{
    Q_OBJECT

public:
    explicit SlcanTransport(const QString &portName,
                            qint32 baudRate = 1000000,
                            QObject *parent = nullptr);

    void start() override;
    void stop() override;

private slots:
    void onReadyRead();

private:
    bool parseLine(const QByteArray &line, CanardCANFrame *frame) const;

    QString m_portName;
    qint32 m_baudRate;
    QSerialPort m_serialPort;
    QByteArray m_lineBuffer;
};
