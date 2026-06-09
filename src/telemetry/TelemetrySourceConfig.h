#pragma once

#include <QtGlobal>

#include <QString>

enum class SourceKind {
    Fake,
    Playback,
    Mavlink,
    DroneCan
};

struct SourceConfig {
    SourceKind kind = SourceKind::Fake;
    QString playbackPath;
    QString mavlinkHost = QStringLiteral("0.0.0.0");
    quint16 mavlinkPort = 14550;
    QString dronecanPort;
};

QString sourceLabelFor(const SourceConfig &config);
