#pragma once

// Compatibility forwarding header. ESPressio-Timing already owns the public
// domain header name ESPressio_SystemClock.hpp, so new System/platform code
// must use ESPressio_SystemPlatformClock.hpp to remain unambiguous when both
// packages participate in one dependency graph.
#include "ESPressio_SystemPlatformClock.hpp"
