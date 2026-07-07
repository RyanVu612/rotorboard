#pragma once

#include "IByteStreamTransport.h"
#include "MavlinkParser.h"
#include "TelemetrySource.h"

#include <QTimer>

#include <functional>

class MavlinkTelemetrySource : public TelemetrySource
{
    Q_OBJECT

public:
    // transport may be nullptr (bypasses transport; use processBytes directly for testing).
    // When transport has no parent, this source takes ownership via setParent.
    explicit MavlinkTelemetrySource(IByteStreamTransport *transport,
                                    QObject *parent = nullptr);

    void start() override;
    void stop() override;

    // Feed raw bytes directly into the parser; exposed for testing (bypasses transport).
    void processBytes(const QByteArray &data);

    static void handleEscStatusMessage(const mavlink_message_t &message,
                                       const std::function<void(const MotorTelemetry &)> &emitSample);
    static void handleBatteryStatusMessage(const mavlink_message_t &message,
                                           const std::function<void(const BatteryTelemetry &)> &emitSample);

private slots:
    void onBytesReceived(const QByteArray &data);
    void onTransportOpened(const QString &endpointName);
    void onTransportOpenFailed();
    void onStatusTick();

private:
    void handleMessage(const mavlink_message_t &message);
    void updateState();
    void setState(LinkState state);
    void emitStatus();

    IByteStreamTransport *m_transport;
    MavlinkParser m_parser;
    QTimer m_statusTimer;

    LinkState m_state = LinkState::NoData;
    bool m_transportOpen = false;
    QString m_endpointName;
    qint64 m_lastByteMillis = 0;
    qint64 m_lastFrameMillis = 0;
    int m_framesSinceTick = 0;
    double m_messageRate = 0.0;
};
