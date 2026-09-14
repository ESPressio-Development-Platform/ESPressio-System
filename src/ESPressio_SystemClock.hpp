#pragma once

// Historical compatibility header.
//
// New System/platform code must include ESPressio_SystemPlatformClock.hpp.
// New Timing-domain code must include ESPressio_TimingSystemClock.hpp.
//
// During the coordinated migration, existing consumers may still include the
// shared historical name. Always expose the System platform clock contract and,
// when ESPressio-Timing is present, also expose its disciplined SystemClock so
// include-directory ordering cannot silently select only one of the two APIs.
#include "ESPressio_SystemPlatformClock.hpp"

#if __has_include(<ESPressio_TimingSystemClock.hpp>)
#include <ESPressio_TimingSystemClock.hpp>
#endif
