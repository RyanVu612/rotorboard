#include "model/WarningLevel.h"
#include "store/MotorTelemetryModel.h"

#include <QTest>

class MotorTelemetryModelTest : public QObject
{
    Q_OBJECT

private slots:
    void createsRowsDynamically();
    void updatesExistingRows();
    void marksRowsStale();
    void exposesWarningLevels();
    void accumulatesHistoryRoles();
};

namespace {
MotorTelemetry sampleForMotor(int motorId)
{
    MotorTelemetry telemetry;
    telemetry.motorId = motorId;
    telemetry.rpm = 2400.0;
    telemetry.voltage = 48.0;
    telemetry.current = 20.0;
    telemetry.temperatureCelsius = 45.0;
    telemetry.pwm = 1500.0;
    telemetry.status = QStringLiteral("OK");
    telemetry.timestampMillis = 1000;
    return telemetry;
}

int warningLevelAt(const MotorTelemetryModel &model, int row)
{
    return model.data(model.index(row), MotorTelemetryModel::WarningLevelRole).toInt();
}
}

void MotorTelemetryModelTest::createsRowsDynamically()
{
    MotorTelemetryModel model;

    model.updateTelemetry(sampleForMotor(3));
    model.updateTelemetry(sampleForMotor(7));

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.data(model.index(0), MotorTelemetryModel::MotorIdRole).toInt(), 3);
    QCOMPARE(model.data(model.index(1), MotorTelemetryModel::MotorIdRole).toInt(), 7);
}

void MotorTelemetryModelTest::updatesExistingRows()
{
    MotorTelemetryModel model;
    MotorTelemetry telemetry = sampleForMotor(2);
    model.updateTelemetry(telemetry);

    telemetry.rpm = 3600.0;
    telemetry.current = 55.0;
    model.updateTelemetry(telemetry);

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0), MotorTelemetryModel::RpmRole).toDouble(), 3600.0);
    QCOMPARE(model.data(model.index(0), MotorTelemetryModel::CurrentRole).toDouble(), 55.0);
}

void MotorTelemetryModelTest::marksRowsStale()
{
    MotorTelemetryModel model;
    model.updateTelemetry(sampleForMotor(1));

    model.refreshStaleState(3001, 2000);

    QCOMPARE(model.data(model.index(0), MotorTelemetryModel::IsStaleRole).toBool(), true);
    QCOMPARE(warningLevelAt(model, 0), static_cast<int>(WarningLevel::Stale));
}

void MotorTelemetryModelTest::exposesWarningLevels()
{
    MotorTelemetryModel model;

    MotorTelemetry warningTelemetry = sampleForMotor(1);
    warningTelemetry.temperatureCelsius = 70.0;
    model.updateTelemetry(warningTelemetry);
    QCOMPARE(warningLevelAt(model, 0), static_cast<int>(WarningLevel::Warning));

    MotorTelemetry criticalTelemetry = sampleForMotor(2);
    criticalTelemetry.current = 125.0;
    model.updateTelemetry(criticalTelemetry);
    QCOMPARE(warningLevelAt(model, 1), static_cast<int>(WarningLevel::Critical));

    MotorTelemetry okTelemetry = sampleForMotor(3);
    model.updateTelemetry(okTelemetry);
    QCOMPARE(warningLevelAt(model, 2), static_cast<int>(WarningLevel::Ok));
}

void MotorTelemetryModelTest::accumulatesHistoryRoles()
{
    MotorTelemetryModel model;

    MotorTelemetry first = sampleForMotor(1);
    first.rpm = 1000.0;
    first.current = 10.0;
    first.temperatureCelsius = 40.0;
    model.updateTelemetry(first);

    MotorTelemetry second = first;
    second.rpm = 2000.0;
    second.current = 20.0;
    second.temperatureCelsius = 50.0;
    model.updateTelemetry(second);

    const QVariantList rpmHistory = model.data(model.index(0), MotorTelemetryModel::RpmHistoryRole).toList();
    const QVariantList currentHistory = model.data(model.index(0), MotorTelemetryModel::CurrentHistoryRole).toList();
    const QVariantList temperatureHistory = model.data(model.index(0), MotorTelemetryModel::TemperatureHistoryRole).toList();

    QCOMPARE(rpmHistory.size(), 2);
    QCOMPARE(rpmHistory.at(0).toDouble(), 1000.0);
    QCOMPARE(rpmHistory.at(1).toDouble(), 2000.0);
    QCOMPARE(currentHistory.at(0).toDouble(), 10.0);
    QCOMPARE(currentHistory.at(1).toDouble(), 20.0);
    QCOMPARE(temperatureHistory.at(0).toDouble(), 40.0);
    QCOMPARE(temperatureHistory.at(1).toDouble(), 50.0);
}

QTEST_MAIN(MotorTelemetryModelTest)
#include "MotorTelemetryModelTest.moc"
