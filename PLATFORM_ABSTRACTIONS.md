# Platform Abstractions Audit Trail

This file records changes made during the platform-abstraction tranche tracked by issue #4.

## 2026-08-27

### Platform vocabulary
- Added `PlatformStatus` and `PlatformResult` so higher-level libraries can report platform failures without exposing native SDK result types.
- Added `ProcessorAffinity`, representing either any processor or a requested specific processor/core without assuming affinity support on every target.

### Execution
- Added the `System::Execution` provider contract for execution creation, destruction, suspension, resumption, current-execution identity, stack telemetry, sleeping and yielding.
- Added a portable processor-count query so higher-level scheduling code does not inspect RTOS core-count macros.
- Execution handles are opaque ESPressio values rather than native RTOS handles.

### Synchronization
- Added a binary `ISignal` abstraction suitable for task lifecycle coordination.
- The contract supports normal and interrupt-context signalling without exposing an RTOS semaphore type.

### Bounded queues
- Added a fixed-element `IMessageQueue` abstraction with bounded capacity, timeout-aware send/receive, reset and queue depth reporting.
- Added explicit interrupt-context sending so callback/ISR producers do not need to expose a native RTOS queue API.

### Clocking
- Added a platform-neutral monotonic clock contract with a portable `std::chrono::steady_clock` fallback.
- Added a high-resolution counter/provider contract for hardware-counter facilities such as ESP32 GPTimer without naming or exposing the underlying implementation.

### GPIO and interrupts
- Added ESPressio-native GPIO pin, state, direction and pull concepts.
- Added edge/level interrupt trigger concepts.
- Added `InterruptConfiguration` with optional processor affinity.
- Added a move-only RAII `InterruptHandle`; destroying the handle destroys/detaches the registered interrupt through the concrete provider.
- Added explicit enable/disable lifecycle operations and provider capability reporting for interrupts and interrupt affinity.
- Refined interrupt creation to return both `PlatformResult` and the owned handle so unsupported/conflicting affinity is observable rather than reduced to a null handle.

### Entropy
- Added `System::Entropy::IEntropySource` for hardware/platform entropy without exposing target RNG APIs.
- Entropy sources declare whether they are suitable for cryptographic use, allowing Security to reject inappropriate fallback/random implementations.
- The default/null source reports unavailable rather than silently substituting a weak pseudo-random generator.

### Byte I/O
- Added `System::IO::IByteInput`, `IByteOutput` and `IByteStream` for raw byte-oriented device/framework streams.
- Added small text/line convenience operations on `IByteOutput` so developer-facing libraries can preserve formatted console/log output without depending on Arduino `Print`.
- The abstraction intentionally does not model serial framing, console parsing or logging policy; those remain domain concerns.

### Umbrella API
- Updated `ESPressio_System.hpp` to expose execution, synchronization, queues, clocking, GPIO, entropy and byte-I/O alongside memory policies.

## Boundary rule

ESPressio-System defines generic hardware/runtime capabilities in ESPressio vocabulary. Domain-specific concepts such as WiFi lifecycle, persistence semantics, ESP-NOW behaviour, event dispatch, socket protocols or command routing remain owned by their domain libraries. Concrete target implementations belong in platform libraries such as ESPressio-ESP32.
