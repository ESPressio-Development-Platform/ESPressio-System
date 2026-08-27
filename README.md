# ESPressio System

Platform-neutral system abstractions shared by ESPressio libraries.

**Release target:** `0.1.0`

ESPressio System exists so higher-level libraries can express system-level requirements without depending directly on ESP32, ESP-IDF, Arduino, PSRAM APIs, or another platform implementation. The first abstraction provided is a portable memory-policy layer.

## When to use it

Most applications do **not** need to call ESPressio System directly. Add it explicitly when you are writing an ESPressio library, providing a platform implementation, or you want allocator-aware containers in application code.

Use `ESPressio-ESP32` alongside this library on ESP32 when you want `ExternalPreferred` or `ExternalRequired` allocations to use PSRAM.

## Memory policies

```cpp
#include <ESPressio_Memory.hpp>

using ESPressio::System::Memory::MemoryPolicy;
```

The available policies are:

| Policy | Meaning |
| --- | --- |
| `Automatic` | Let the installed platform provider choose its normal allocation strategy. |
| `Internal` | Request internal/system memory. |
| `ExternalPreferred` | Prefer external memory, but permit the provider to fall back when it cannot satisfy the request externally. |
| `ExternalRequired` | Require external memory; failure is reported through allocation failure if the provider cannot satisfy it. |

The built-in default provider uses normal C++ allocation. This keeps System and libraries that depend on it usable in host tests and on platforms that do not provide a specialised memory implementation.

## Allocator-aware containers

System supplies STL-compatible allocators and aliases:

```cpp
using namespace ESPressio::System::Memory;

Vector<int, MemoryPolicy::ExternalPreferred> samples;
Deque<Job, MemoryPolicy::ExternalPreferred> jobs;
Map<int, Record, MemoryPolicy::ExternalPreferred> records;
UnorderedMap<Key, Value, MemoryPolicy::ExternalPreferred> lookup;
String<MemoryPolicy::ExternalPreferred> text;

auto shared = MakeShared<MyObject, MemoryPolicy::ExternalPreferred>(constructorArg);
```

The aliases retain ordinary STL semantics. The memory policy applies to storage obtained through that allocator; nested members that perform their own allocations still follow their own allocator rules.

## Installing a platform provider

A platform integration implements `IMemoryProvider` and installs it with:

```cpp
System::Memory::SetProvider(&provider);
```

`ResetProvider()` restores the portable default provider.

### Provider lifetime and construction order

This is important: `Allocator<T, Policy>` captures the **currently installed provider when the allocator is constructed**. It keeps that provider so the same implementation that allocated memory can later deallocate it safely.

Therefore, if application-global ESPressio objects contain `ExternalPreferred` storage, install the platform provider **before those objects are constructed**. On ESP32, ESPressio-ESP32 provides the platform implementation and documents an early-bootstrap pattern.

Changing the global provider later affects newly constructed allocators; it does not retarget already-constructed allocator objects.

## Writing a provider

Implement:

```cpp
class MyProvider : public ESPressio::System::Memory::IMemoryProvider {
public:
    void* Allocate(
        std::size_t bytes,
        std::size_t alignment,
        MemoryPolicy policy
    ) override;

    void Deallocate(
        void* pointer,
        std::size_t bytes,
        std::size_t alignment,
        MemoryPolicy policy
    ) noexcept override;

    bool Supports(MemoryPolicy policy) const noexcept override;
};
```

A provider is responsible for honouring the requested alignment and for defining the platform meaning of each policy. Higher-level ESPressio libraries must not bypass this abstraction with platform-specific allocation APIs.

## Installation during coordinated development

Until the staged ESPressio release cascade is complete, consume the active working branch:

```ini
lib_deps =
    https://github.com/ESPressio-Development-Platform/ESPressio-System.git#feature/1-system-memory-policy
```

After `0.1.0` is released, applications should use the normal released package/version instead of a development branch.

## Design guarantees

- Header-only platform-neutral memory abstraction.
- No ESP32, Arduino, FreeRTOS or ESP-IDF dependency.
- No RTTI requirement.
- Allocators remember their provider for correct deallocation.
- Platform-specific behaviour belongs in a platform implementation such as ESPressio-ESP32.
- `ExternalPreferred` is a policy request, not a guarantee; `ExternalRequired` is the strict form.

## Testing

The working branch has host regression coverage for provider installation, policy propagation and allocator/container behaviour. Higher-level ESPressio CI also consumes this branch with RTTI disabled.

See `OPTIMISATIONS.md` for the chronological memory-policy implementation history and `CHANGELOG.md` for release-facing changes.
