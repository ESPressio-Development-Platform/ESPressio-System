# Synchronization and Queues

## Binary signals

System exposes `ISignal` for lifecycle handshakes and callback/interrupt-to-execution-context signalling.

```cpp
auto signal = ESPressio::System::Synchronization::CreateBinarySignal();
```

Signals support normal signalling, interrupt-context signalling, timeout-aware waiting, and reset without exposing native semaphore/event types.

## Bounded queues

System also exposes fixed-element bounded queues for cross-context message passing.

```cpp
auto queue = ESPressio::System::Queue::Create<MyMessage>(8);
```

Queues support timeout-aware send/receive, zero-timeout non-blocking operation, interrupt-context producers, reset, capacity, and size inspection.

The primitive queue contract copies fixed-size message values. Higher-level ownership and move semantics belong to the consuming domain rather than being hidden behind an RTOS-specific primitive.