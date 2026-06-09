#include "CsvPlaybackSource.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>
#include <QVector>
#include <algorithm>

CsvPlaybackSource::CsvPlaybackSource(QString csvPath, QObject *parent)
    : TelemetrySource(parent)
{
    m_timer.setInterval(kIntervalMs);
    connect(&m_timer, &QTimer::timeout, this, &CsvPlaybackSource::tick);
    m_csvPath = std::move(csvPath);
}

void CsvPlaybackSource::loadCsv()
{
    QFile csvFile(m_csvPath);
    if (!csvFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open CSV file" << m_csvPath;
        return;
    }

    QTextStream csvStream(&csvFile);
    
    csvStream.readLine(); // skip header line
    while (!csvStream.atEnd()) {
        const QString line = csvStream.readLine();
        const QStringList fields = line.split(',');
        if (fields.size() != 8) {
            continue;
        }
        TimedSample timedSample;
        timedSample.playbackTimeMillis = fields[0].toLongLong();
        timedSample.telemetry.motorId = fields[1].toInt();
        timedSample.telemetry.rpm = fields[2].toDouble();
        timedSample.telemetry.voltage = fields[3].toDouble();
        timedSample.telemetry.current = fields[4].toDouble();
        timedSample.telemetry.temperatureCelsius = fields[5].toDouble();
        timedSample.telemetry.pwm = fields[6].toDouble();
        timedSample.telemetry.status = fields[7];
        m_samples.append(timedSample);
    }

    std::sort(m_samples.begin(), m_samples.end(),
                [](const TimedSample &a, const TimedSample &b) {
                    return a.playbackTimeMillis < b.playbackTimeMillis;
                });

    csvFile.close();
    qDebug() << "Loaded" << m_samples.size() << "samples from" << m_csvPath;
}

void CsvPlaybackSource::start()
{
    m_nextSampleIndex = 0;
    m_startedAtMillis = QDateTime::currentMSecsSinceEpoch();
    m_samples.clear();
    loadCsv();
    m_timer.start();
}

void CsvPlaybackSource::stop()
{
    m_timer.stop();
}

void CsvPlaybackSource::tick()
{
    const qint64 elapsedMillis = QDateTime::currentMSecsSinceEpoch() - m_startedAtMillis;

    while (m_nextSampleIndex < m_samples.size() &&
           m_samples[m_nextSampleIndex].playbackTimeMillis <= elapsedMillis) {
        emit telemetryReceived(m_samples[m_nextSampleIndex].telemetry);
        ++m_nextSampleIndex;
    }

    if (m_nextSampleIndex >= m_samples.size()) {
        stop();
    }
}