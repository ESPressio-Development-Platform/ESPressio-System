#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "ESPressio_Memory.hpp"
#include "ESPressio_Platform.hpp"
#include "ESPressio_Synchronization.hpp"

namespace ESPressio {
namespace System {
namespace Queue {

/// <summary>Abstracts a fixed-element-size platform message queue.</summary>
class IMessageQueue {
public:
    virtual ~IMessageQueue() = default;

    /// <summary>Sends one element to the queue, optionally waiting for capacity.</summary>
    /// <param name="item">Pointer to an element whose size matches <c>ElementSize()</c>.</param>
    /// <param name="timeoutMilliseconds">Maximum time to wait for queue capacity.</param>
    virtual PlatformResult Send(
        const void* item,
        uint32_t timeoutMilliseconds = 0
    ) noexcept = 0;

    /// <summary>Sends one element using an interrupt-safe platform path.</summary>
    virtual PlatformResult SendFromInterrupt(const void* item) noexcept = 0;

    /// <summary>Receives one element from the queue, optionally waiting for data.</summary>
    /// <param name="item">Destination buffer for the received element.</param>
    /// <param name="timeoutMilliseconds">Maximum time to wait for an element.</param>
    virtual PlatformResult Receive(
        void* item,
        uint32_t timeoutMilliseconds = Synchronization::WaitForever
    ) noexcept = 0;

    /// <summary>Removes all queued elements and restores the queue to its empty state.</summary>
    virtual PlatformResult Reset() noexcept = 0;

    /// <summary>Gets the byte size of each queue element.</summary>
    virtual std::size_t ElementSize() const noexcept = 0;
    /// <summary>Gets the maximum number of elements the queue can hold.</summary>
    virtual std::size_t Capacity() const noexcept = 0;
    /// <summary>Gets the current number of queued elements.</summary>
    virtual std::size_t Size() const noexcept = 0;
};

/// <summary>Creates platform-backed message queues with a specified element size and capacity.</summary>
class IQueueProvider {
public:
    virtual ~IQueueProvider() = default;

    /// <summary>Creates a message queue using the provider's normal/default memory placement.</summary>
    /// <param name="elementSize">Size in bytes of each queue element.</param>
    /// <param name="capacity">Maximum number of elements retained by the queue.</param>
    virtual std::unique_ptr<IMessageQueue> Create(
        std::size_t elementSize,
        std::size_t capacity
    ) = 0;

    /// <summary>Creates a message queue using an explicit memory-placement policy when supported by the platform.</summary>
    /// <param name="elementSize">Size in bytes of each queue element.</param>
    /// <param name="capacity">Maximum number of elements retained by the queue.</param>
    /// <param name="policy">Requested backing-storage placement policy.</param>
    /// <returns>The created queue, or null when the requested policy cannot be supported.</returns>
    /// <remarks>The default implementation preserves compatibility with providers that predate policy-aware queues: Automatic and Internal requests use normal creation while external requests report unavailable.</remarks>
    virtual std::unique_ptr<IMessageQueue> Create(
        std::size_t elementSize,
        std::size_t capacity,
        Memory::MemoryPolicy policy
    ) {
        if (
            policy == Memory::MemoryPolicy::Automatic ||
            policy == Memory::MemoryPolicy::Internal
        ) {
            return Create(elementSize, capacity);
        }
        return nullptr;
    }
};

inline IQueueProvider*& ProviderStorage() noexcept {
    static IQueueProvider* provider = nullptr;
    return provider;
}

/// <summary>Gets the currently installed queue provider, or null when none is configured.</summary>
inline IQueueProvider* Provider() noexcept {
    return ProviderStorage();
}

/// <summary>Installs the process-wide queue provider.</summary>
inline void SetProvider(IQueueProvider* provider) noexcept {
    ProviderStorage() = provider;
}

/// <summary>Removes the currently installed queue provider.</summary>
inline void ResetProvider() noexcept {
    ProviderStorage() = nullptr;
}

/// <summary>Creates a queue through the active provider using its normal/default memory placement.</summary>
/// <param name="elementSize">Size in bytes of each queue element.</param>
/// <param name="capacity">Maximum number of elements retained by the queue.</param>
/// <returns>The created queue, or null when no provider is installed.</returns>
inline std::unique_ptr<IMessageQueue> Create(
    std::size_t elementSize,
    std::size_t capacity
) {
    auto* provider = Provider();
    return provider != nullptr ? provider->Create(elementSize, capacity) : nullptr;
}

/// <summary>Creates a queue through the active provider using an explicit memory-placement policy.</summary>
/// <param name="elementSize">Size in bytes of each queue element.</param>
/// <param name="capacity">Maximum number of elements retained by the queue.</param>
/// <param name="policy">Requested backing-storage placement policy.</param>
/// <returns>The created queue, or null when no provider is installed or the policy cannot be satisfied.</returns>
inline std::unique_ptr<IMessageQueue> Create(
    std::size_t elementSize,
    std::size_t capacity,
    Memory::MemoryPolicy policy
) {
    auto* provider = Provider();
    return provider != nullptr
        ? provider->Create(elementSize, capacity, policy)
        : nullptr;
}

/// <summary>Creates a typed queue whose element size is inferred from <typeparamref name="T"/>.</summary>
/// <typeparam name="T">Element type stored in the queue.</typeparam>
/// <param name="capacity">Maximum number of elements retained by the queue.</param>
template<typename T>
inline std::unique_ptr<IMessageQueue> Create(std::size_t capacity) {
    return Create(sizeof(T), capacity);
}

/// <summary>Creates a typed queue whose backing storage follows an explicit memory policy.</summary>
/// <typeparam name="T">Element type stored in the queue.</typeparam>
/// <param name="capacity">Maximum number of elements retained by the queue.</param>
/// <param name="policy">Requested backing-storage placement policy.</param>
template<typename T>
inline std::unique_ptr<IMessageQueue> Create(
    std::size_t capacity,
    Memory::MemoryPolicy policy
) {
    return Create(sizeof(T), capacity, policy);
}

}
}
}
