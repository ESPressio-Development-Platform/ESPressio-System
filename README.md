# ESPressio System

Platform-neutral hardware and runtime concepts expressed in the language of the ESPressio platform.

**Release target:** `0.1.0`

ESPressio System is the hardware/runtime abstraction boundary at the base of the ESPressio dependency graph. Higher-level libraries can express requirements such as memory policy, execution, clocking, synchronization, bounded queues and GPIO without depending directly on ESP32, ESP-IDF, Arduino, FreeRTOS or another target implementation.

System deliberately does **not** own higher-level domain concepts such as WiFi lifecycle, ESP-NOW transport, event dispatch or command routing. Those abstractions remain with their respective libraries. Concrete target implementations belong in platform libraries such as `ESPressio-ESP32`.

## When to use it

Most applications do not need to interact with every System interface directly. Add it explicitly when you are:

- writing an ESPressio library that needs a hardware/runtime capability;
- providing a target-platform implementation;
- using allocator-aware containers directly;
- consuming GPIO or another System capability directly from application code.

On ESP32, use `ESPressio-ESP32` alongside this library to install the concrete providers.

## Platform result and processor affinity

`ESPressio_Platform.hpp` provides common platform vocabulary:

```cpp
using ESPressio::System::PlatformResult;
using ESPressio::System::PlatformStatus;
using ESPressio::System::ProcessorAffinity;
```

`PlatformResult` keeps native SDK error types out of higher-level APIs while retaining an optional integer native diagnostic code. `ProcessorAffinity` can represent either any processor or a requested specific processor/core without assuming every target supports affinity.

## Memory policies

```cpp
#include <ESPressio_Memory.hpp>

using ESPressio::System::Memory::MemoryPolicy;
```

| Policy | Meaning |
| --- | --- |
| `Automatic` | Let the installed platform provider choose its normal allocation strategy. |
| `Internal` | Request internal/system memory. |
| `ExternalPreferred` | Prefer external memory, but permit provider fallback. |
| `ExternalRequired` | Require external memory. |

The built-in default provider uses normal C++ allocation so host tests and non-specialised platforms remain usable.

System supplies STL-compatible allocator aliases including `Vector`, `Deque`, `Map`, `UnorderedMap`, `String` and `MakeShared`.

Memory providers are installed with:

```cpp
System::Memory::SetProvider(&provider);
```

Allocator objects capture the active provider when constructed so allocation and deallocation remain paired correctly. Install a specialised provider before constructing allocator-aware global objects.

## Execution

`ESPressio_Execution.hpp` defines the primitive execution capability beneath higher-level libraries such as ESPressio-Task and ESPressio-Threads.

The contract covers:

- execution creation and destruction;
- suspend and resume;
- current execution identity;
- stack high-water/free-stack telemetry;
- processor-count discovery;
- sleep and yield;
- optional processor affinity.

Execution handles are opaque ESPressio values. Native RTOS task handles must not leak through this API.

A platform implementation installs an `IExecutionProvider` with:

```cpp
System::Execution::SetProvider(&provider);
```

The provider reports `ProcessorCount()` separately from `SupportsProcessorAffinity()`: a platform may expose multiple processors while still being unable to guarantee the requested execution-placement semantics.

## Synchronization signals

`ESPressio_Synchronization.hpp` exposes a binary `ISignal` suitable for lifecycle handshakes and callback/interrupt-to-task signalling.

```cpp
auto signal = ESPressio::System::Synchronization::CreateBinarySignal();
```

Signals support ordinary signalling, interrupt-context signalling, timeout-aware waiting and reset. Native semaphore/event types remain inside the platform implementation.

## Bounded message queues

`ESPressio_Queue.hpp` exposes a fixed-element bounded queue abstraction for cross-execution-context message passing.

```cpp
auto queue = ESPressio::System::Queue::Create<MyMessage>(8);
```

The queue supports:

- timeout-aware `Send()` and `Receive()`;
- non-blocking operation with a zero timeout;
- `SendFromInterrupt()` for ISR/callback producers;
- reset, capacity and current-size inspection.

The queue intentionally works in terms of fixed element size and copied message values. Higher-level ownership/move semantics remain the responsibility of the consuming domain rather than being hidden behind an RTOS-specific queue contract.

## Monotonic clocks and high-resolution counters

`ESPressio_Clock.hpp` separates two related concepts:

- `IMonotonicClock` provides monotonically increasing nanosecond timestamps;
- `IHighResolutionCounter` represents a dedicated high-resolution hardware counter with start/stop/reset/read lifecycle.

A portable `std::chrono::steady_clock` monotonic fallback is provided for host use. Hardware-target providers can install a more appropriate monotonic clock and a high-resolution counter provider.

This distinction allows Timing and other libraries to request the semantic capability they need without knowing whether ESP32 `esp_timer`, GPTimer, or another target facility provides it.

## GPIO

`ESPressio_GPIO.hpp` defines ESPressio-native concepts for:

- pin identity;
- input/output/open-drain direction;
- pull-up/down configuration;
- digital state;
- edge and level interrupt triggers;
- interrupt lifecycle;
- optional processor affinity.

Example:

```cpp
#include <ESPressio_GPIO.hpp>

using namespace ESPressio::System::GPIO;

auto* gpio = Controller();
if (gpio != nullptr) {
    gpio->Configure(26, {Direction::Output, Pull::None, State::Low});
    gpio->Write(26, State::High);
}
```

### Interrupt lifecycle

Interrupt creation returns an explicit status together with a move-only RAII handle:

```cpp
auto created = gpio->CreateInterrupt(
    26,
    {InterruptTrigger::RisingEdge, ProcessorAffinity::Any(), true},
    callback,
    context
);

if (created) {
    auto interrupt = std::move(created.Handle);
    // interrupt->Disable();
    // interrupt->Enable();
}
```

The handle owns the registration. Destroying/resetting the handle detaches and destroys the interrupt through the concrete provider. `Enable()` and `Disable()` allow the registration to remain owned while temporarily inactive.

Specific processor affinity is a request, not a universal guarantee. Providers expose `SupportsInterruptAffinity()` and may return `Unsupported` or `Conflict` when a requested affinity cannot be satisfied. Returning `InterruptCreationResult` ensures those causes are not collapsed into an unexplained null handle.

## Provider ownership rule

System owns abstractions for hardware/runtime capabilities that make sense independently of a higher-level feature domain. Feature libraries own their own contextual platform interfaces.

For example:

- memory, clocks, GPIO, primitive execution, synchronization and bounded queues belong in System;
- `IWiFiPlatform` belongs in ESPressio-WiFi;
- ESP32 implementations of both System and WiFi contracts belong in ESPressio-ESP32.

This keeps System from becoming a catch-all service library while preserving a clean dependency-inversion boundary.

## Installation during coordinated development

```ini
lib_deps =
    https://github.com/ESPressio-Development-Platform/ESPressio-System.git#feature/1-system-memory-policy
```

After the coordinated release is published, applications should use the released version instead of the development branch.

## Design guarantees

- No ESP32, Arduino, FreeRTOS or ESP-IDF dependency in the abstraction layer.
- No RTTI requirement.
- Native platform handles/types are hidden behind ESPressio vocabulary.
- Higher-level domain abstractions remain owned by their domain libraries.
- Concrete hardware/runtime behaviour belongs in a platform implementation such as ESPressio-ESP32.
- Existing memory-policy semantics remain preserved.

## Auditing and testing

`PLATFORM_ABSTRACTIONS.md` records the platform-abstraction tranche chronologically. Host regression coverage validates memory-provider behaviour and the portable platform contracts; provider-specific repositories validate the target implementations.

See `OPTIMISATIONS.md` for the chronological memory-policy implementation history and `CHANGELOG.md` for release-facing changes.
