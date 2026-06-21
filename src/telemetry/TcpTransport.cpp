#include "TcpTransport.h"

static constexpr int kReconnectMs = 3000;

TcpTransport::TcpTransport(const QString &host, quint16 port, QObject *parent)
    : IByteStreamTransport(parent)
    , m_host(host)
    , m_port(port)
{
    connect(&m_socket, &QTcpSocket::connected, this, &TcpTransport::onConnected);
    connect(&m_socket, &QTcpSocket::readyRead, this, &TcpTransport::onReadyRead);
    connect(&m_socket, &QTcpSocket::errorOccurred, this, &TcpTransport::onErrorOccurred);
    connect(&m_socket, &QTcpSocket::disconnected, this, &TcpTransport::onDisconnected);

    m_reconnectTimer.setSingleShot(true);
    m_reconnectTimer.setInterval(kReconnectMs);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &TcpTransport::attemptConnect);
}

void TcpTransport::start()
{
    m_started = true;
    attemptConnect();
}

void TcpTransport::stop()
{
    m_started = false;
    m_reconnectTimer.stop();
    m_socket.abort(); // fires disconnected/errorOccurred synchronously; m_started=false so slots bail
    m_connected = false;
}

void TcpTransport::attemptConnect()
{
    // Abort any in-flight attempt before re-trying (prevents stacked attempts on slow DNS/timeout)
    m_socket.abort();
    m_connected = false;
    m_socket.connectToHost(m_host, m_port);
}

void TcpTransport::onConnected()
{
    m_connected = true;
    m_reconnectTimer.stop();
    emit opened(QStringLiteral("%1:%2").arg(m_host).arg(m_port));
}

void TcpTransport::onReadyRead()
{
    emit bytesReceived(m_socket.readAll());
}

void TcpTransport::onErrorOccurred(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    if (!m_started || m_connected) {
        return;
    }
    emit openFailed();
    m_reconnectTimer.start();
}

void TcpTransport::onDisconnected()
{
    if (!m_started || !m_connected) {
        return;
    }
    m_connected = false;
    emit openFailed();
    m_reconnectTimer.start();
}
