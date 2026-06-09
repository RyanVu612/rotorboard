#include "TelemetrySourceFactory.h"

#include "CsvPlaybackSource.h"
#include "DroneCanTelemetrySource.h"
#include "FakeTelemetrySource.h"
#include "MavlinkTelemetrySource.h"

#include <memory>

std::unique_ptr<TelemetrySource> makeTelemetrySource(const SourceConfig &config)
{
    switch (config.kind) {
    case SourceKind::Playback:
        return std::make_unique<CsvPlaybackSource>(config.playbackPath);
    case SourceKind::Mavlink:
        return std::make_unique<MavlinkTelemetrySource>(config.mavlinkHost, config.mavlinkPort);
    case SourceKind::DroneCan:
        return std::make_unique<DroneCanTelemetrySource>(config.dronecanPort);
    case SourceKind::Fake:
    default:
        return std::make_unique<FakeTelemetrySource>();
    }
}

QString sourceLabelFor(const SourceConfig &config)
{
    switch (config.kind) {
    case SourceKind::Playback:
        return QStringLiteral("Playback");
    case SourceKind::Mavlink:
        return QStringLiteral("MAVLink UDP %1:%2").arg(config.mavlinkHost).arg(config.mavlinkPort);
    case SourceKind::DroneCan:
        if (config.dronecanPort.isEmpty()) {
            return QStringLiteral("DroneCAN (no port)");
        }
        return QStringLiteral("DroneCAN %1").arg(config.dronecanPort);
    case SourceKind::Fake:
    default:
        return QStringLiteral("Fake source");
    }
}
