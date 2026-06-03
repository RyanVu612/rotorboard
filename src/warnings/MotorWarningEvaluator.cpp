#include "MotorWarningEvaluator.h"

namespace {
constexpr double kCriticalTemperatureCelsius = 80.0;
constexpr double kWarningTemperatureCelsius = 65.0;
constexpr double kCriticalCurrentAmps = 120.0;
constexpr double kWarningCurrentAmps = 80.0;
constexpr double kWarningVoltageVolts = 42.0;
}

WarningLevel MotorWarningEvaluator::evaluate(const MotorTelemetry &telemetry, bool isStale) const
{
    if (isStale) {
        return WarningLevel::Stale;
    }

    if (telemetry.temperatureCelsius > kCriticalTemperatureCelsius ||
        telemetry.current > kCriticalCurrentAmps) {
        return WarningLevel::Critical;
    }

    if (telemetry.temperatureCelsius > kWarningTemperatureCelsius ||
        telemetry.current > kWarningCurrentAmps ||
        telemetry.voltage < kWarningVoltageVolts) {
        return WarningLevel::Warning;
    }

    return WarningLevel::Ok;
}
