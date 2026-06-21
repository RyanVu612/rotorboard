#pragma once

#include "IByteStreamTransport.h"

#include <QUdpSocket>

class UdpTransport : public IByteStreamTransport
{
    Q_OBJECT

public:
    explicit UdpTransport(const QString &host = QStringLiteral("0.0.0.0"),
                          quint16 port = 14550,
                          QObject *parent = nullptr);

    void start() override;
    void stop() override;

private slots:
    void onReadyRead();

private:
    QString m_host;
    quint16 m_port;
    QUdpSocket m_socket;
};
