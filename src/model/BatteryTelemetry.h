#pragma once

#include <QMetaType>
#include <QVector>
#include <QtGlobal>

struct BatteryTelemetry {
    int batteryId = 0;
    double voltage = 0.0;
    double current = 0.0;
    double batteryRemaining = -1.0;
    double temperatureCelsius = 0.0;
    double currentConsumedMah = 0.0;
    QVector<double> cellVoltages;
    qint64 timestampMillis = 0;
};

Q_DECLARE_METATYPE(BatteryTelemetry)
