# ESPressio-System

Platform-neutral system abstractions for ESPressio libraries.

## Memory policy

`ESPressio_Memory.hpp` provides `MemoryPolicy`, `IMemoryProvider`, provider installation, STL-compatible allocators and container aliases. Higher-level ESPressio libraries depend only on these abstractions; platform applications install an appropriate provider (for ESP32, from ESPressio-ESP32).

Policies: `Automatic`, `Internal`, `ExternalPreferred`, and `ExternalRequired`.

The default provider uses normal C++ allocation and makes the abstraction portable to host/non-ESP32 builds.
