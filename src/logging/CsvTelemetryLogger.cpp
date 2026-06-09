#include "logging/CsvTelemetryLogger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

namespace {

QString defaultLogDirectory()
{
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QString buildDirName = appDir.dirName();
    if (buildDirName == QLatin1String("build") || buildDirName == QLatin1String("build2")) {
        return QDir(appDir.absoluteFilePath(QStringLiteral(".."))).absoluteFilePath(QStringLiteral("logs"));
    }
    return QDir(QDir::currentPath()).absoluteFilePath(QStringLiteral("logs"));
}

} // namespace

QString CsvTelemetryLogger::resolveLogFilePath(const QString &filePath)
{
    const QString logDir = defaultLogDirectory();

    if (filePath.isEmpty()) {
        const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
        return QDir(logDir).absoluteFilePath(QStringLiteral("session-%1.csv").arg(timestamp));
    }

    const QFileInfo info(filePath);
    if (info.path().isEmpty() || info.path() == QLatin1String(".")) {
        return QDir(logDir).absoluteFilePath(info.fileName());
    }

    return filePath;
}

bool CsvTelemetryLogger::start(const QString &filePath)
{
    stop();

    const QString resolvedPath = resolveLogFilePath(filePath);
    QDir().mkpath(QFileInfo(resolvedPath).absolutePath());

    m_file.setFileName(resolvedPath);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    m_stream.setDevice(&m_file);
    m_stream << kHeaderRow << '\n';
    m_stream.flush();
    m_active = true;
    return true;
}

void CsvTelemetryLogger::stop()
{
    if (!m_active) {
        return;
    }

    m_stream.flush();
    m_file.close();
    m_active = false;
}

bool CsvTelemetryLogger::isActive() const
{
    return m_active;
}

void CsvTelemetryLogger::logSample(const MotorTelemetry &telemetry, qint64 sessionElapsedMillis)
{
    if (!m_active) {
        return;
    }

    m_stream << sessionElapsedMillis << ','
             << telemetry.motorId << ','
             << telemetry.rpm << ','
             << telemetry.voltage << ','
             << telemetry.current << ','
             << telemetry.temperatureCelsius << ','
             << telemetry.pwm << ','
             << telemetry.status << '\n';
    m_stream.flush();
}
