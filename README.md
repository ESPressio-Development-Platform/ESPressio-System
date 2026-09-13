# ESPressio-System

`ESPressio-System` is the absolute dependency root of the ESPressio Development Platform.

It has exactly two responsibilities:

1. canonical device/runtime identity semantics; and
2. the domain-neutral compile-time Composition Framework used by higher ESPressio domains.

## Dependency rule

`ESPressio-System` must never declare or consume any other ESPressio library dependency.

Higher libraries depend downward on System. System never depends upward on Platform, Primitive, Task, Threads, Serializable, Persistence, Security, OTA, Radio, Mesh, or any other ESPressio domain.

## Identity

System owns:

- `DeviceIdentifier` — stable 128-bit device identity;
- `RuntimeIncarnationId` — non-zero 32-bit identity for one actual process/runtime boot;
- `DeviceRuntimeIdentity` — `{DeviceIdentifier, RuntimeIncarnationId}`;
- `RuntimeIdentity` — process-wide immutable publication of the installed runtime identity.

Persistence may allocate and durably commit a runtime incarnation before installing it into `RuntimeIdentity`, but System has no Persistence dependency and cannot inspect persistence.

## Composition Framework

The reusable framework is exposed in:

```cpp
ESPressio::System::CompositionFramework
```

The framework owns only generic compile-time mechanics:

- domain identity;
- exclusive/shared capability categories;
- capability profiles and sets;
- properties and property constraints;
- requirement sets;
- provider declarations;
- provider lists;
- composition validation and provider resolution.

Semantic domains define their own domain tags and re-export the framework vocabulary under their own namespaces. For example, normal consumers should use `Platform::Composition<>` or `OTA::Composition<>`, not manually assemble another domain's composition.

Domain identity is part of every framework contract. A capability or requirement from one domain cannot satisfy another domain's composition accidentally.

`ProviderListFor<Capability>` exposes all providers for a shared capability in declaration order. That order is structural only and conveys no priority, failover, scheduling, or policy meaning.

`ProviderFor<Capability>` is valid only when exactly one provider supplies the capability.

## What System no longer owns

Platform/SDK abstractions are owned by `ESPressio-Platform`, including execution, synchronization, queueing, memory, clock, entropy, GPIO, byte streams and related platform-provider mechanisms.

Those former System abstractions are intentionally absent from this baseline.
