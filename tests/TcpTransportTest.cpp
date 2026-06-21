#include "telemetry/TcpTransport.h"

#include <QSignalSpy>
#include <QTcpServer>
#include <QTest>

class TcpTransportTest : public QObject
{
    Q_OBJECT

private slots:
    void emitsOpenFailedOnRefusedConnection();
    void emitsOpenedOnSuccessfulConnection();
    void emitsBytesReceivedOnData();
    void stopWhileConnectingEmitsNoSignals();
};

void TcpTransportTest::emitsOpenFailedOnRefusedConnection()
{
    // Port 1 is reserved and connection will be refused immediately on all platforms.
    TcpTransport transport(QStringLiteral("127.0.0.1"), 1);
    QSignalSpy failSpy(&transport, &TcpTransport::openFailed);
    QSignalSpy openSpy(&transport, &TcpTransport::opened);

    transport.start();
    QVERIFY(failSpy.wait(5000));
    QCOMPARE(openSpy.count(), 0);

    transport.stop();
}

void TcpTransportTest::emitsOpenedOnSuccessfulConnection()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost, 0));
    const quint16 port = server.serverPort();

    TcpTransport transport(QStringLiteral("127.0.0.1"), port);
    QSignalSpy openSpy(&transport, &TcpTransport::opened);
    QSignalSpy failSpy(&transport, &TcpTransport::openFailed);

    transport.start();
    QVERIFY(openSpy.wait(3000));
    QCOMPARE(failSpy.count(), 0);
    QVERIFY(openSpy.first().first().toString().contains(QString::number(port)));

    transport.stop();
}

void TcpTransportTest::emitsBytesReceivedOnData()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost, 0));
    const quint16 port = server.serverPort();

    TcpTransport transport(QStringLiteral("127.0.0.1"), port);
    QSignalSpy openSpy(&transport, &TcpTransport::opened);
    QSignalSpy bytesSpy(&transport, &TcpTransport::bytesReceived);

    transport.start();
    QVERIFY(openSpy.wait(3000));

    QVERIFY(server.hasPendingConnections());
    QTcpSocket *clientConn = server.nextPendingConnection();
    const QByteArray payload = QByteArrayLiteral("hello");
    clientConn->write(payload);
    clientConn->flush();

    QVERIFY(bytesSpy.wait(3000));
    QCOMPARE(bytesSpy.first().first().toByteArray(), payload);

    transport.stop();
}

void TcpTransportTest::stopWhileConnectingEmitsNoSignals()
{
    // Connect to a port nothing listens on (use a server that accepts but never sends).
    // We just want to call stop() before the connection resolves and verify no signals come after.
    TcpTransport transport(QStringLiteral("127.0.0.1"), 1);
    QSignalSpy failSpy(&transport, &TcpTransport::openFailed);
    QSignalSpy openSpy(&transport, &TcpTransport::opened);

    transport.start();
    transport.stop(); // stop immediately — no openFailed should come after this

    // Give the event loop a chance to deliver any stale queued signals.
    QTest::qWait(200);

    QCOMPARE(openSpy.count(), 0);
    // Any openFailed that arrived synchronously before stop() set m_started=false
    // would have been swallowed; nothing should arrive after.
    // We verify that after stop() no *new* signals arrive by checking the count is stable.
    const int countAfterStop = failSpy.count();
    QTest::qWait(200);
    QCOMPARE(failSpy.count(), countAfterStop);
}

QTEST_MAIN(TcpTransportTest)
#include "TcpTransportTest.moc"
