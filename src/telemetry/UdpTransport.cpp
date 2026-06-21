#include "UdpTransport.h"

#include <QDebug>
#include <QHostAddress>
#include <QNetworkDatagram>

UdpTransport::UdpTransport(const QString &host, quint16 port, QObject *parent)
    : IByteStreamTransport(parent)
    , m_host(host)
    , m_port(port)
{
    connect(&m_socket, &QUdpSocket::readyRead, this, &UdpTransport::onReadyRead);
}

void UdpTransport::start()
{
    QHostAddress bindAddress;
    if (m_host.isEmpty() || m_host == QLatin1String("0.0.0.0")) {
        bindAddress = QHostAddress::AnyIPv4;
    } else {
        bindAddress.setAddress(m_host);
    }

    if (!m_socket.bind(bindAddress, m_port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qWarning() << "UdpTransport failed to bind" << bindAddress << m_port << m_socket.errorString();
        emit openFailed();
        return;
    }

    emit opened(QStringLiteral("%1:%2").arg(m_host).arg(m_port));
}

void UdpTransport::stop()
{
    m_socket.close();
}

void UdpTransport::onReadyRead()
{
    while (m_socket.hasPendingDatagrams()) {
        const QNetworkDatagram datagram = m_socket.receiveDatagram();
        emit bytesReceived(datagram.data());
    }
}
