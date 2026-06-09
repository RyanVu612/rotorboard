#include "logging/CsvTelemetryLogger.h"
#include "model/MotorTelemetry.h"
#include "telemetry/CsvPlaybackSource.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

class CsvTelemetryLoggerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void resolveLogFilePathUsesDefaultDirectory();
    void writesHeaderAndRows();
    void outputMatchesCsvPlaybackSourceParser();
};

namespace {

MotorTelemetry sampleTelemetry(int motorId, double rpm, double current, const QString &status)
{
    MotorTelemetry telemetry;
    telemetry.motorId = motorId;
    telemetry.rpm = rpm;
    telemetry.voltage = 48.0;
    telemetry.current = current;
    telemetry.temperatureCelsius = 45.0;
    telemetry.pwm = 1500.0;
    telemetry.status = status;
    telemetry.timestampMillis = 1000;
    return telemetry;
}

QStringList readCsvLines(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll()).split('\n', Qt::SkipEmptyParts);
}

} // namespace

void CsvTelemetryLoggerTest::initTestCase()
{
    qRegisterMetaType<MotorTelemetry>();
}

void CsvTelemetryLoggerTest::resolveLogFilePathUsesDefaultDirectory()
{
    const QString defaultPath = CsvTelemetryLogger::resolveLogFilePath(QString());
    QVERIFY(defaultPath.endsWith(QStringLiteral(".csv")));
    QVERIFY(defaultPath.contains(QStringLiteral("session-")));
    QVERIFY(defaultPath.contains(QStringLiteral("logs")));

    const QString relativePath = CsvTelemetryLogger::resolveLogFilePath(QStringLiteral("custom.csv"));
    QVERIFY(relativePath.endsWith(QStringLiteral("logs/custom.csv"))
             || relativePath.endsWith(QStringLiteral("logs\\custom.csv")));

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString explicitPath = tempDir.filePath(QStringLiteral("explicit.csv"));
    QCOMPARE(CsvTelemetryLogger::resolveLogFilePath(explicitPath), explicitPath);
}

void CsvTelemetryLoggerTest::writesHeaderAndRows()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString filePath = tempDir.filePath(QStringLiteral("session.csv"));
    CsvTelemetryLogger logger;
    QVERIFY(logger.start(filePath));

    logger.logSample(sampleTelemetry(1, 2400.0, 20.0, QStringLiteral("OK")), 0);
    logger.logSample(sampleTelemetry(2, 3600.0, 55.0, QStringLiteral("WARN")), 100);
    logger.stop();

    const QStringList lines = readCsvLines(filePath);
    QVERIFY(!lines.isEmpty());
    QCOMPARE(lines.size(), 3);
    QCOMPARE(lines.at(0),
             QStringLiteral("playbackTimeMillis,motorId,rpm,voltage,current,temperatureCelsius,pwm,status"));
    QCOMPARE(lines.at(1), QStringLiteral("0,1,2400,48,20,45,1500,OK"));
    QCOMPARE(lines.at(2), QStringLiteral("100,2,3600,48,55,45,1500,WARN"));
}

void CsvTelemetryLoggerTest::outputMatchesCsvPlaybackSourceParser()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString filePath = tempDir.filePath(QStringLiteral("session.csv"));
    CsvTelemetryLogger logger;
    QVERIFY(logger.start(filePath));

    const MotorTelemetry first = sampleTelemetry(3, 1200.0, 15.0, QStringLiteral("OK"));
    const MotorTelemetry second = sampleTelemetry(4, 4200.0, 90.0, QStringLiteral("CRIT"));
    logger.logSample(first, 0);
    logger.logSample(second, 250);
    logger.stop();

    CsvPlaybackSource playbackSource(filePath);
    QSignalSpy spy(&playbackSource, &CsvPlaybackSource::telemetryReceived);
    playbackSource.start();

    QTRY_COMPARE(spy.count(), 2);

    const auto firstSample = qvariant_cast<MotorTelemetry>(spy.at(0).at(0));
    QCOMPARE(firstSample.motorId, first.motorId);
    QCOMPARE(firstSample.rpm, first.rpm);
    QCOMPARE(firstSample.voltage, first.voltage);
    QCOMPARE(firstSample.current, first.current);
    QCOMPARE(firstSample.temperatureCelsius, first.temperatureCelsius);
    QCOMPARE(firstSample.pwm, first.pwm);
    QCOMPARE(firstSample.status, first.status);

    const auto secondSample = qvariant_cast<MotorTelemetry>(spy.at(1).at(0));
    QCOMPARE(secondSample.motorId, second.motorId);
    QCOMPARE(secondSample.rpm, second.rpm);
    QCOMPARE(secondSample.voltage, second.voltage);
    QCOMPARE(secondSample.current, second.current);
    QCOMPARE(secondSample.temperatureCelsius, second.temperatureCelsius);
    QCOMPARE(secondSample.pwm, second.pwm);
    QCOMPARE(secondSample.status, second.status);

    playbackSource.stop();
}

QTEST_MAIN(CsvTelemetryLoggerTest)
#include "CsvTelemetryLoggerTest.moc"
