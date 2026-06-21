#pragma once

#include <QtGlobal>

#include <QString>

enum class SourceKind {
    Fake,
    Playback,
    Mavlink,
    MavlinkSerial,
    MavlinkTcp,
    DroneCan
};

struct SourceConfig {
    SourceKind kind = SourceKind::Fake;
    QString playbackPath;
    QString mavlinkHost = QStringLiteral("0.0.0.0");
    quint16 mavlinkPort = 14550;
    QString mavlinkSerialPort;
    qint32 mavlinkSerialBaud = 115200;
    QString mavlinkTcpHost = QStringLiteral("127.0.0.1");
    quint16 mavlinkTcpPort = 5760;
    QString dronecanPort;
};

QString sourceLabelFor(const SourceConfig &config);
