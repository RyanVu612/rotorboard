#include "app/AppController.h"

#include "model/MotorTelemetry.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main (int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    qRegisterMetaType<MotorTelemetry>();

    QString logPath;
    bool logRequested = false;
    QString playbackPath;
    const QStringList args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        if (args.at(i) == QLatin1String("--log")) {
            logRequested = true;
            if (i + 1 < args.size() && !args.at(i + 1).startsWith(QLatin1Char('-'))) {
                logPath = args.at(++i);
            }
        } else if (args.at(i) == QLatin1String("--playback") && i + 1 < args.size()) {
            playbackPath = args.at(++i);
        }
    }

    AppController controller(playbackPath);
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
