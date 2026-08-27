#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "ESPressio_Platform.hpp"
#include "ESPressio_Synchronization.hpp"

namespace ESPressio {
namespace System {
namespace Queue {

class IMessageQueue {
public:
    virtual ~IMessageQueue() = default;

    virtual PlatformResult Send(
        const void* item,
        uint32_t timeoutMilliseconds = 0
    ) noexcept = 0;

    virtual PlatformResult SendFromInterrupt(const void* item) noexcept = 0;

    virtual PlatformResult Receive(
        void* item,
        uint32_t timeoutMilliseconds = Synchronization::WaitForever
    ) noexcept = 0;

    virtual PlatformResult Reset() noexcept = 0;

    virtual std::size_t ElementSize() const noexcept = 0;
    virtual std::size_t Capacity() const noexcept = 0;
    virtual std::size_t Size() const noexcept = 0;
};

class IQueueProvider {
public:
    virtual ~IQueueProvider() = default;

    virtual std::unique_ptr<IMessageQueue> Create(
        std::size_t elementSize,
        std::size_t capacity
    ) = 0;
};

inline IQueueProvider*& ProviderStorage() noexcept {
    static IQueueProvider* provider = nullptr;
    return provider;
}

inline IQueueProvider* Provider() noexcept {
    return ProviderStorage();
}

inline void SetProvider(IQueueProvider* provider) noexcept {
    ProviderStorage() = provider;
}

inline void ResetProvider() noexcept {
    ProviderStorage() = nullptr;
}

inline std::unique_ptr<IMessageQueue> Create(
    std::size_t elementSize,
    std::size_t capacity
) {
    auto* provider = Provider();
    return provider != nullptr ? provider->Create(elementSize, capacity) : nullptr;
}

template<typename T>
inline std::unique_ptr<IMessageQueue> Create(std::size_t capacity) {
    return Create(sizeof(T), capacity);
}

}
}
}
