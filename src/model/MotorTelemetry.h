#pragma once

#include <QMetaType>
#include <QString>
#include <QtGlobal>

struct MotorTelemetry {
    int motorId = 0;
    double rpm = 0.0;
    double voltage = 0.0;
    double current = 0.0;
    double temperatureCelsius = 0.0;
    double pwm = 0.0;
    QString status;
    qint64 timestampMillis = 0;
};

Q_DECLARE_METATYPE(MotorTelemetry)