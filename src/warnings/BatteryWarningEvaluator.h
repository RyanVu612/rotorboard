#pragma once

#include "model/BatteryTelemetry.h"
#include "model/WarningLevel.h"

class BatteryWarningEvaluator
{
public:
    WarningLevel evaluate(const BatteryTelemetry &telemetry, bool isStale) const;
};
