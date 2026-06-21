#pragma once

#include "IByteStreamTransport.h"

#include <QTcpSocket>
#include <QTimer>

class TcpTransport : public IByteStreamTransport
{
    Q_OBJECT

public:
    explicit TcpTransport(const QString &host = QStringLiteral("127.0.0.1"),
                          quint16 port = 5760,
                          QObject *parent = nullptr);

    void start() override;
    void stop() override;

private slots:
    void onConnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void onDisconnected();
    void attemptConnect();

private:
    QString m_host;
    quint16 m_port;
    QTcpSocket m_socket;
    QTimer m_reconnectTimer;
    bool m_started = false;
    bool m_connected = false;
};
