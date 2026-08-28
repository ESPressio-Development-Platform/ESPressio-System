# Memory Policies

ESPressio System 1.0.0 provides allocator-aware memory placement without exposing target-specific allocation APIs to consuming libraries.

```cpp
#include <ESPressio_Memory.hpp>

using ESPressio::System::Memory::MemoryPolicy;
```

## Policies

| Policy | Intent |
| --- | --- |
| `Automatic` | Let the active provider select its normal allocation strategy. |
| `Internal` | Request internal/system memory. |
| `ExternalPreferred` | Prefer external memory while allowing provider fallback. |
| `ExternalRequired` | Require external memory. |

A platform may expose different physical memory classes while consumers continue to use the same ESPressio vocabulary.

## Allocator-aware containers

System provides STL-compatible aliases including:

```cpp
using namespace ESPressio::System::Memory;

Vector<MyType> values;
Deque<MyType> pending;
String<> text;
```

`Vector`, `Deque`, `Map`, `UnorderedMap`, `String`, and `MakeShared` can all carry an explicit `MemoryPolicy` template argument.

For example:

```cpp
Vector<Sample, MemoryPolicy::ExternalPreferred> history;

auto object = MakeShared<MyObject, MemoryPolicy::Internal>(constructorArgument);
```

## Provider capture is important

An `Allocator` captures the active `IMemoryProvider` when the allocator is constructed. Allocation and deallocation therefore remain paired with the same provider even if the globally installed provider later changes.

Consequently, install the target-specific memory provider **before constructing allocator-aware global or long-lived objects**.

## Installing a provider

```cpp
using namespace ESPressio::System::Memory;

IMemoryProvider* previous = SetProvider(&myProvider);
```

`SetProvider(nullptr)` and `ResetProvider()` restore the portable default provider.

The default provider uses normal C++ allocation and makes host testing possible without a specialised target implementation.

## For platform implementers

If you are adding support for another MCU, RTOS, board family, or memory architecture, continue with [Memory Provider Contract](Memory-Provider-Contract).
