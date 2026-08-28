# API Overview

ESPressio System 1.0.0 groups portable runtime and hardware capabilities behind ESPressio-native interfaces.

| Area | Purpose |
| --- | --- |
| Platform | Common result/status and processor-affinity vocabulary |
| Memory | Memory policies, provider, STL-compatible allocators and containers |
| Execution | Primitive execution lifecycle and telemetry |
| Synchronization | Signals and portable synchronization primitives |
| Queue | Bounded fixed-element message queues |
| Clock | Monotonic clocks and high-resolution counters |
| GPIO | Digital I/O and interrupt lifecycle |
| Byte Stream | Portable byte-oriented stream abstraction |
| Entropy | Platform-neutral entropy capability |
| System Clock | Platform-facing system-clock facilities |

## Execution defaults

`ExecutionConfiguration` defaults to the name `ESPressio`, a 4096-byte stack, priority `1`, and unrestricted processor affinity. Platform implementations may reject unsupported requested semantics explicitly.

## Provider fallbacks

System deliberately includes safe fallback/null-provider behaviour where appropriate so that the abstraction layer remains host-testable and does not acquire a native SDK dependency.

Use the dedicated Wiki pages for behavioural guidance. Source headers remain the authoritative declarations of exact 1.0.0 signatures.