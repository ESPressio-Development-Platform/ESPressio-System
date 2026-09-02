# Memory Provider Contract

This page is intended for developers adding a concrete platform implementation beneath ESPressio System 1.0.0.

A memory provider implements:

```cpp
class IMemoryProvider {
public:
    virtual ~IMemoryProvider() = default;

    virtual void* Allocate(
        std::size_t bytes,
        std::size_t alignment,
        MemoryPolicy policy
    ) = 0;

    virtual void Deallocate(
        void* pointer,
        std::size_t bytes,
        std::size_t alignment,
        MemoryPolicy policy
    ) noexcept = 0;

    virtual bool Supports(MemoryPolicy policy) const noexcept = 0;
};
```

## Responsibilities

The provider translates ESPressio's semantic policies into the target's actual memory facilities.

A provider must preserve:

1. **Alignment requirements.** `Allocate()` receives the required alignment for the allocated type.
2. **Policy semantics.** `ExternalRequired` must not silently become an unrestricted allocation merely because that is convenient for the target implementation.
3. **Allocation/deallocation symmetry.** Memory must be returned through the mechanism appropriate to the allocation that produced it.
4. **Stable provider lifetime.** Allocator instances retain a pointer to the provider that existed when they were constructed. The provider must therefore outlive every allocator/container that captured it.
5. **No platform-type leakage.** Consumers interact only with `IMemoryProvider`, `MemoryPolicy`, and ESPressio allocator/container types.

## `ExternalPreferred` versus `ExternalRequired`

These policies deliberately express different contracts.

`ExternalPreferred` permits a target provider to attempt external memory and fall back to another valid allocation class.

`ExternalRequired` expresses a hard requirement. If the target cannot satisfy it, allocation must fail according to the provider's allocation semantics rather than silently violating the requested placement.

## Installing the provider

A platform integration normally installs its provider during early platform initialisation:

```cpp
ESPressio::System::Memory::SetProvider(&provider);
```

Do this before allocator-aware globals or long-lived services are constructed wherever the target's startup architecture permits it.

## Testing an implementation

At minimum, platform-provider tests should verify:

- every advertised policy in `Supports()` can actually be allocated;
- required alignment is honoured;
- deallocation uses the correct memory facility;
- `ExternalPreferred` fallback behaves as documented;
- `ExternalRequired` cannot silently fall back;
- allocator instances remain valid if the global provider is subsequently replaced;
- zero-sized allocation requests are handled safely.

See also [Testing Platform Providers](Testing-Platform-Providers).
