#pragma once

#include "MavlinkParser.h"
#include "TelemetrySource.h"

#include <QHostAddress>
#include <QUdpSocket>

#include <functional>

class MavlinkTelemetrySource : public TelemetrySource
{
    Q_OBJECT

public:
    explicit MavlinkTelemetrySource(const QString &host = QStringLiteral("0.0.0.0"),
                                    quint16 port = 14550,
                                    QObject *parent = nullptr);

    void start() override;
    void stop() override;

    static void handleEscStatusMessage(const mavlink_message_t &message,
                                       const std::function<void(const MotorTelemetry &)> &emitSample);

private slots:
    void onReadyRead();

private:
    void handleMessage(const mavlink_message_t &message);

    QString m_host;
    quint16 m_port = 14550;
    QUdpSocket m_socket;
    MavlinkParser m_parser;
};
