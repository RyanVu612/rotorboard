#include "model/WarningLevel.h"
#include "store/BatteryTelemetryModel.h"

#include <QTest>

class BatteryTelemetryModelTest : public QObject
{
    Q_OBJECT

private slots:
    void createsRowsDynamically();
    void updatesExistingRows();
    void marksRowsStale();
    void exposesWarningLevels();
    void excludesUnknownValuesFromWarnings();
    void accumulatesHistoryRoles();
};

namespace {

BatteryTelemetry sampleForBattery(int batteryId)
{
    BatteryTelemetry telemetry;
    telemetry.batteryId = batteryId;
    telemetry.voltage = 22.2;
    telemetry.current = 12.5;
    telemetry.batteryRemaining = 75.0;
    telemetry.temperatureCelsius = 31.0;
    telemetry.currentConsumedMah = 600.0;
    telemetry.cellVoltages = {3.7, 3.71, 3.69, 3.72, 3.70, 3.71};
    telemetry.timestampMillis = 1000;
    return telemetry;
}

int warningLevelAt(const BatteryTelemetryModel &model, int row)
{
    return model.data(model.index(row), BatteryTelemetryModel::WarningLevelRole).toInt();
}

} // namespace

void BatteryTelemetryModelTest::createsRowsDynamically()
{
    BatteryTelemetryModel model;

    model.updateTelemetry(sampleForBattery(0));
    model.updateTelemetry(sampleForBattery(2));

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.data(model.index(0), BatteryTelemetryModel::BatteryIdRole).toInt(), 0);
    QCOMPARE(model.data(model.index(1), BatteryTelemetryModel::BatteryIdRole).toInt(), 2);
}

void BatteryTelemetryModelTest::updatesExistingRows()
{
    BatteryTelemetryModel model;
    BatteryTelemetry telemetry = sampleForBattery(1);
    model.updateTelemetry(telemetry);

    telemetry.voltage = 24.5;
    telemetry.current = 18.0;
    telemetry.batteryRemaining = 65.0;
    model.updateTelemetry(telemetry);

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0), BatteryTelemetryModel::VoltageRole).toDouble(), 24.5);
    QCOMPARE(model.data(model.index(0), BatteryTelemetryModel::CurrentRole).toDouble(), 18.0);
    QCOMPARE(model.valueForMetric(1, QStringLiteral("batteryRemaining")).toDouble(), 65.0);
    QCOMPARE(model.cellVoltagesForBattery(1).size(), 6);
}

void BatteryTelemetryModelTest::marksRowsStale()
{
    BatteryTelemetryModel model;
    model.updateTelemetry(sampleForBattery(3));

    model.refreshStaleState(3001, 2000);

    QCOMPARE(model.data(model.index(0), BatteryTelemetryModel::IsStaleRole).toBool(), true);
    QCOMPARE(warningLevelAt(model, 0), static_cast<int>(WarningLevel::Stale));
}

void BatteryTelemetryModelTest::exposesWarningLevels()
{
    BatteryTelemetryModel model;

    BatteryTelemetry warningTelemetry = sampleForBattery(1);
    warningTelemetry.batteryRemaining = 15.0;
    model.updateTelemetry(warningTelemetry);

    BatteryTelemetry criticalTelemetry = sampleForBattery(2);
    criticalTelemetry.cellVoltages = {3.65, 3.29, 3.66};
    model.updateTelemetry(criticalTelemetry);

    BatteryTelemetry okTelemetry = sampleForBattery(3);
    model.updateTelemetry(okTelemetry);

    QCOMPARE(warningLevelAt(model, 0), static_cast<int>(WarningLevel::Warning));
    QCOMPARE(warningLevelAt(model, 1), static_cast<int>(WarningLevel::Critical));
    QCOMPARE(warningLevelAt(model, 2), static_cast<int>(WarningLevel::Ok));
}

void BatteryTelemetryModelTest::excludesUnknownValuesFromWarnings()
{
    BatteryTelemetryModel model;

    BatteryTelemetry unknownTelemetry = sampleForBattery(1);
    unknownTelemetry.batteryRemaining = -1.0;
    unknownTelemetry.cellVoltages.clear();
    model.updateTelemetry(unknownTelemetry);

    BatteryTelemetry criticalTelemetry = sampleForBattery(2);
    criticalTelemetry.batteryRemaining = 5.0;
    criticalTelemetry.cellVoltages.clear();
    model.updateTelemetry(criticalTelemetry);

    QCOMPARE(warningLevelAt(model, 0), static_cast<int>(WarningLevel::Ok));
    QCOMPARE(warningLevelAt(model, 1), static_cast<int>(WarningLevel::Critical));
}

void BatteryTelemetryModelTest::accumulatesHistoryRoles()
{
    BatteryTelemetryModel model;

    BatteryTelemetry first = sampleForBattery(1);
    first.voltage = 22.0;
    first.current = 10.0;
    model.updateTelemetry(first);

    BatteryTelemetry second = first;
    second.voltage = 24.0;
    second.current = 20.0;
    model.updateTelemetry(second);

    const QVariantList voltageHistory = model.data(model.index(0), BatteryTelemetryModel::VoltageHistoryRole).toList();
    const QVariantList currentHistory = model.data(model.index(0), BatteryTelemetryModel::CurrentHistoryRole).toList();

    QCOMPARE(voltageHistory.size(), 2);
    QCOMPARE(voltageHistory.at(0).toDouble(), 22.0);
    QCOMPARE(voltageHistory.at(1).toDouble(), 24.0);
    QCOMPARE(currentHistory.size(), 2);
    QCOMPARE(currentHistory.at(0).toDouble(), 10.0);
    QCOMPARE(currentHistory.at(1).toDouble(), 20.0);
}

QTEST_MAIN(BatteryTelemetryModelTest)
#include "BatteryTelemetryModelTest.moc"
