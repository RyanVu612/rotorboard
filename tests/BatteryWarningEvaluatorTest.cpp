#include "warnings/BatteryWarningEvaluator.h"

#include <QTest>

class BatteryWarningEvaluatorTest : public QObject
{
    Q_OBJECT

private slots:
    void returnsOkForHealthyTelemetry();
    void warnsForLowRemaining();
    void warnsForLowCellVoltage();
    void criticalForVeryLowRemaining();
    void criticalForVeryLowCellVoltage();
    void staleOverridesOtherFields();
    void excludesUnknownRemaining();
    void excludesUnknownCellVoltages();
};

namespace {

BatteryTelemetry healthyTelemetry()
{
    BatteryTelemetry telemetry;
    telemetry.batteryRemaining = 75.0;
    telemetry.cellVoltages = {3.7, 3.71, 3.69};
    return telemetry;
}

} // namespace

void BatteryWarningEvaluatorTest::returnsOkForHealthyTelemetry()
{
    BatteryWarningEvaluator evaluator;
    QCOMPARE(evaluator.evaluate(healthyTelemetry(), false), WarningLevel::Ok);
}

void BatteryWarningEvaluatorTest::warnsForLowRemaining()
{
    BatteryWarningEvaluator evaluator;
    BatteryTelemetry telemetry = healthyTelemetry();
    telemetry.batteryRemaining = 19.0;
    QCOMPARE(evaluator.evaluate(telemetry, false), WarningLevel::Warning);
}

void BatteryWarningEvaluatorTest::warnsForLowCellVoltage()
{
    BatteryWarningEvaluator evaluator;
    BatteryTelemetry telemetry = healthyTelemetry();
    telemetry.cellVoltages = {3.6, 3.49, 3.62};
    QCOMPARE(evaluator.evaluate(telemetry, false), WarningLevel::Warning);
}

void BatteryWarningEvaluatorTest::criticalForVeryLowRemaining()
{
    BatteryWarningEvaluator evaluator;
    BatteryTelemetry telemetry = healthyTelemetry();
    telemetry.batteryRemaining = 9.0;
    QCOMPARE(evaluator.evaluate(telemetry, false), WarningLevel::Critical);
}

void BatteryWarningEvaluatorTest::criticalForVeryLowCellVoltage()
{
    BatteryWarningEvaluator evaluator;
    BatteryTelemetry telemetry = healthyTelemetry();
    telemetry.cellVoltages = {3.6, 3.29, 3.62};
    QCOMPARE(evaluator.evaluate(telemetry, false), WarningLevel::Critical);
}

void BatteryWarningEvaluatorTest::staleOverridesOtherFields()
{
    BatteryWarningEvaluator evaluator;
    BatteryTelemetry telemetry = healthyTelemetry();
    telemetry.batteryRemaining = 5.0;
    QCOMPARE(evaluator.evaluate(telemetry, true), WarningLevel::Stale);
}

void BatteryWarningEvaluatorTest::excludesUnknownRemaining()
{
    BatteryWarningEvaluator evaluator;
    BatteryTelemetry telemetry = healthyTelemetry();
    telemetry.batteryRemaining = -1.0;
    QCOMPARE(evaluator.evaluate(telemetry, false), WarningLevel::Ok);
}

void BatteryWarningEvaluatorTest::excludesUnknownCellVoltages()
{
    BatteryWarningEvaluator evaluator;
    BatteryTelemetry telemetry = healthyTelemetry();
    telemetry.cellVoltages.clear();
    QCOMPARE(evaluator.evaluate(telemetry, false), WarningLevel::Ok);
}

QTEST_MAIN(BatteryWarningEvaluatorTest)
#include "BatteryWarningEvaluatorTest.moc"
