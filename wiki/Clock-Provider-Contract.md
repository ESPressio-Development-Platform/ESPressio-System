# Clock Provider Contract

Clock providers preserve the distinction between a general monotonic timestamp source and a dedicated high-resolution counter.

## Monotonic clock

Implement `IMonotonicClock` with a source that never moves backwards and exposes timestamps using ESPressio's nanosecond semantics.

## High-resolution counter

A high-resolution counter implementation must honour its explicit lifecycle and provide the dedicated counter semantics promised by the interface.

Do not collapse both abstractions merely because one native timer API happens to be capable of serving both roles on a particular MCU.

## Native facilities

Selection of facilities such as an SDK monotonic timer or dedicated hardware timer is a platform implementation decision and must remain invisible to consumers.