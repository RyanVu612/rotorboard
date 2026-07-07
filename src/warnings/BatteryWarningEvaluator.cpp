#include "BatteryWarningEvaluator.h"

#include <algorithm>
#include <limits>

WarningLevel BatteryWarningEvaluator::evaluate(const BatteryTelemetry &telemetry, bool isStale) const
{
    if (isStale) {
        return WarningLevel::Stale;
    }

    const bool remainingKnown = telemetry.batteryRemaining >= 0.0;
    double minCellVoltage = std::numeric_limits<double>::infinity();
    for (double cell : telemetry.cellVoltages) {
        minCellVoltage = std::min(minCellVoltage, cell);
    }
    const bool hasCellVoltage = !telemetry.cellVoltages.isEmpty();

    if ((remainingKnown && telemetry.batteryRemaining < 10.0) ||
        (hasCellVoltage && minCellVoltage < 3.3)) {
        return WarningLevel::Critical;
    }
    if ((remainingKnown && telemetry.batteryRemaining < 20.0) ||
        (hasCellVoltage && minCellVoltage < 3.5)) {
        return WarningLevel::Warning;
    }
    return WarningLevel::Ok;
}
