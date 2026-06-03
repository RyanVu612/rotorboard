#include "app/AppController.h"

#include "model/MotorTelemetry.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main (int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    qRegisterMetaType<MotorTelemetry>();

    AppController controller;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);
    engine.loadFromModule("Rotorboard", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
