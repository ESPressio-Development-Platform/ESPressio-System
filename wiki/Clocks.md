# Clocks

ESPressio System separates monotonic time from dedicated high-resolution hardware counters.

## Monotonic clock

`IMonotonicClock` provides monotonically increasing nanosecond timestamps suitable for elapsed-time measurement and higher-level timing facilities.

A portable steady-clock fallback supports host environments.

## High-resolution counter

`IHighResolutionCounter` represents a dedicated counter with explicit lifecycle such as start, stop, reset, and read.

Keeping these concepts separate lets a target choose the appropriate facilities—for example a general monotonic timer versus a dedicated hardware peripheral—without exposing those native choices to higher-level libraries.

For implementation guidance see [Clock Provider Contract](Clock-Provider-Contract).