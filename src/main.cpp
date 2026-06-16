#include "app/AppController.h"

#include "model/MotorTelemetry.h"
#include "telemetry/TelemetrySourceConfig.h"

#include <QDebug>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

namespace {

bool parseMavlinkEndpoint(const QString &value, QString *host, quint16 *port)
{
    const int colonIndex = value.lastIndexOf(QLatin1Char(':'));
    if (colonIndex <= 0 || colonIndex >= value.size() - 1) {
        return false;
    }

    bool ok = false;
    const quint16 parsedPort = value.mid(colonIndex + 1).toUShort(&ok);
    if (!ok) {
        return false;
    }

    *host = value.left(colonIndex);
    *port = parsedPort;
    return true;
}

} // namespace

int main (int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Rotorboard"));
    QCoreApplication::setApplicationName(QStringLiteral("Rotorboard"));
    qRegisterMetaType<MotorTelemetry>();

    QString logPath;
    bool logRequested = false;
    SourceConfig sourceConfig;
    bool playbackRequested = false;
    bool mavlinkRequested = false;
    bool mavlinkSerialRequested = false;
    bool dronecanRequested = false;

    const QStringList args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        if (args.at(i) == QLatin1String("--log")) {
            logRequested = true;
            if (i + 1 < args.size() && !args.at(i + 1).startsWith(QLatin1Char('-'))) {
                logPath = args.at(++i);
            }
        } else if (args.at(i) == QLatin1String("--playback") && i + 1 < args.size()) {
            sourceConfig.kind = SourceKind::Playback;
            sourceConfig.playbackPath = args.at(++i);
            playbackRequested = true;
        } else if (args.at(i) == QLatin1String("--mavlink")) {
            sourceConfig.kind = SourceKind::Mavlink;
            mavlinkRequested = true;
            if (i + 1 < args.size() && !args.at(i + 1).startsWith(QLatin1Char('-'))) {
                const QString endpoint = args.at(++i);
                QString host;
                quint16 port = 0;
                if (parseMavlinkEndpoint(endpoint, &host, &port)) {
                    sourceConfig.mavlinkHost = host;
                    sourceConfig.mavlinkPort = port;
                } else {
                    qWarning() << "Invalid --mavlink endpoint" << endpoint << "(expected host:port)";
                }
            }
        } else if (args.at(i) == QLatin1String("--mavlink-serial")) {
            sourceConfig.kind = SourceKind::MavlinkSerial;
            mavlinkSerialRequested = true;
            // Optional [port] [baud]. A lone numeric token is treated as the baud
            // rate (auto-detect the port); otherwise it is the port path,
            // optionally followed by a numeric baud rate.
            if (i + 1 < args.size() && !args.at(i + 1).startsWith(QLatin1Char('-'))) {
                const QString token = args.at(++i);
                bool isNumeric = false;
                const qint32 numericValue = token.toInt(&isNumeric);
                if (isNumeric) {
                    sourceConfig.mavlinkSerialBaud = numericValue;
                } else {
                    sourceConfig.mavlinkSerialPort = token;
                    if (i + 1 < args.size() && !args.at(i + 1).startsWith(QLatin1Char('-'))) {
                        bool baudOk = false;
                        const qint32 baud = args.at(i + 1).toInt(&baudOk);
                        if (baudOk) {
                            sourceConfig.mavlinkSerialBaud = baud;
                            ++i;
                        }
                    }
                }
            }
        } else if (args.at(i) == QLatin1String("--dronecan")) {
            sourceConfig.kind = SourceKind::DroneCan;
            dronecanRequested = true;
            if (i + 1 < args.size() && !args.at(i + 1).startsWith(QLatin1Char('-'))) {
                sourceConfig.dronecanPort = args.at(++i);
            }
        }
    }

    const int liveSourceCount = (playbackRequested ? 1 : 0) + (mavlinkRequested ? 1 : 0) +
                                (mavlinkSerialRequested ? 1 : 0) + (dronecanRequested ? 1 : 0);
    if (liveSourceCount > 1) {
        if (dronecanRequested) {
            qWarning() << "Multiple telemetry sources requested; using DroneCAN source";
            sourceConfig.kind = SourceKind::DroneCan;
        } else if (mavlinkRequested) {
            qWarning() << "Multiple telemetry sources requested; using MAVLink source";
            sourceConfig.kind = SourceKind::Mavlink;
        } else if (mavlinkSerialRequested) {
            qWarning() << "Multiple telemetry sources requested; using MAVLink serial source";
            sourceConfig.kind = SourceKind::MavlinkSerial;
        }
        sourceConfig.playbackPath.clear();
    }

    AppController controller(sourceConfig, liveSourceCount > 0);
    if (logRequested) {
        controller.startLogging(logPath);
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);
    engine.loadFromModule("Rotorboard", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
