#pragma once

#include "TelemetrySourceConfig.h"

#include <memory>

class TelemetrySource;

std::unique_ptr<TelemetrySource> makeTelemetrySource(const SourceConfig &config);
