#include "model/MotorTelemetry.h"
#include "telemetry/FakeTelemetrySource.h"

#include <QCoreApplication>
#include <QDebug>

int main (int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    qRegisterMetaType<MotorTelemetry>();

    FakeTelemetrySource source;
    QObject::connect(&source, &TelemetrySource::telemetryReceived, 
                    &app, [](const MotorTelemetry &t) {
                        qInfo().nospace()
                            << "M" << t.motorId
                            << " rpm=" << t.rpm
                            << " V=" << t.voltage
                            << " A=" << t.current
                            << " T=" << t.temperatureCelsius;
                    });

    source.start();
    return app.exec();
}