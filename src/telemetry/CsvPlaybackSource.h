#pragma once

#include "TelemetrySource.h"

#include <QTimer>
#include <QRandomGenerator>

class CsvPlaybackSource : public TelemetrySource
{
    Q_OBJECT

public:
    explicit CsvPlaybackSource(QString csvPath, QObject *parent = nullptr);

    void start() override;
    void stop() override;

private:
    struct TimedSample {
        qint64 playbackTimeMillis = 0;
        MotorTelemetry telemetry;
    };

    void loadCsv();
    void tick();

    QVector<TimedSample> m_samples;
    qsizetype m_nextSampleIndex = 0;
    qint64 m_startedAtMillis = 0;
    QTimer m_timer;
    QString m_csvPath;
    static constexpr int kIntervalMs = 100;
};