#include "app/AppController.h"

#include "model/MotorTelemetry.h"
#include "telemetry/TelemetrySourceConfig.h"

#include <QDebug>
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
    qRegisterMetaType<MotorTelemetry>();

    QString logPath;
    bool logRequested = false;
    SourceConfig sourceConfig;
    bool playbackRequested = false;
    bool mavlinkRequested = false;

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
        }
    }

    if (playbackRequested && mavlinkRequested) {
        qWarning() << "--playback and --mavlink are mutually exclusive; using MAVLink source";
        sourceConfig.kind = SourceKind::Mavlink;
        sourceConfig.playbackPath.clear();
    }

    AppController controller(sourceConfig);
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
