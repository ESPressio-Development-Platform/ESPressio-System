# Changelog

## 0.1.0 — 2026-08-27

Initial ESPressio System release.

### Added

- Platform-neutral `MemoryPolicy` abstraction with `Automatic`, `Internal`, `ExternalPreferred`, and `ExternalRequired` policies.
- `IMemoryProvider` contract and portable default C++ provider.
- Global provider installation/reset APIs.
- STL-compatible policy allocators and allocator-aware `Vector`, `Deque`, `Map`, `UnorderedMap`, and `String` aliases.
- Provider-aware `MakeShared` helper.
- Host regression coverage and dual ordinary-CMake / ESP-IDF component integration.

### Design

- Higher-level ESPressio libraries depend on System abstractions rather than directly on ESP32 heap APIs.
- Allocators capture their provider at construction so deallocation always returns memory through the provider that allocated it.
- No RTTI, Arduino, FreeRTOS, ESP32, or ESP-IDF dependency is required by System itself.
