#pragma once

#include "model/MotorTelemetry.h"

#include <QFile>
#include <QString>
#include <QTextStream>

class CsvTelemetryLogger
{
public:
    static QString resolveLogFilePath(const QString &filePath);

    bool start(const QString &filePath);
    void stop();
    bool isActive() const;

    void logSample(const MotorTelemetry &telemetry, qint64 sessionElapsedMillis);

private:
    static constexpr const char *kHeaderRow =
        "playbackTimeMillis,motorId,rpm,voltage,current,temperatureCelsius,pwm,status";

    QFile m_file;
    QTextStream m_stream;
    bool m_active = false;
};
