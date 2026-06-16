#pragma once

#include "MavlinkParser.h"
#include "TelemetrySource.h"

#include <QByteArray>
#include <QSerialPort>
#include <QString>
#include <QTimer>

class MavlinkSerialTelemetrySource : public TelemetrySource
{
    Q_OBJECT

public:
    // Diagnostic link states, ordered worst-to-healthy. Numeric values are part
    // of the contract with the UI (used to pick the indicator colour).
    enum LinkState {
        FailedOpen = 0,    // serial port could not be opened (missing/busy/denied)
        NoData = 1,        // port open, no bytes received
        NoValidFrames = 2, // bytes arriving, but none parse as MAVLink
        Receiving = 3      // valid MAVLink frames arriving
    };

    explicit MavlinkSerialTelemetrySource(const QString &portName = QString(),
                                          qint32 baudRate = 115200,
                                          QObject *parent = nullptr);

    void start() override;
    void stop() override;

    // Parse a chunk of raw serial bytes: feeds the MAVLink parser, emits motor
    // telemetry for ESC_STATUS, and updates the link counters. Exposed so tests
    // can exercise the parse path without a real serial port.
    void processBytes(const QByteArray &data);

signals:
    // state is a LinkState value; messageRate is valid MAVLink frames/second;
    // portName is the resolved serial port (empty when none was found).
    void linkStatusChanged(int state, double messageRate, const QString &portName);

private slots:
    void onReadyRead();
    void onStatusTick();

private:
    void handleMessage(const mavlink_message_t &message);
    void updateState();
    void setState(LinkState state);
    void emitStatus();

    QString m_requestedPortName;
    QString m_activePortName;
    qint32 m_baudRate;
    QSerialPort m_serialPort;
    MavlinkParser m_parser;
    QTimer m_statusTimer;

    LinkState m_state = NoData;
    bool m_portOpen = false;
    qint64 m_lastByteMillis = 0;
    qint64 m_lastFrameMillis = 0;
    int m_framesSinceTick = 0;
    double m_messageRate = 0.0;
};
