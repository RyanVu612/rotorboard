#pragma once

#include "model/MotorTelemetry.h"
#include "model/WarningLevel.h"

class MotorWarningEvaluator
{
public:
    WarningLevel evaluate(const MotorTelemetry &telemetry, bool isStale) const;
};
