# Optimisations

Chronological record of system-level memory optimisation work.

## 2026-08-27

- **#1** Introduced platform-neutral `MemoryPolicy` and `IMemoryProvider` abstractions.
- **#1** Added provider installation/reset semantics with a portable default provider.
- **#2** Added STL-compatible policy allocators and allocator-aware Vector, Deque, Map, UnorderedMap and String aliases.
- **#2** Added provider-aware `MakeShared` for long-lived framework objects.
- **#2** Made the root CMake integration portable across ESP-IDF component builds and ordinary host `FetchContent` consumers.
