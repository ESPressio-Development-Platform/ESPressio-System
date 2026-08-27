#pragma once

#include <cstdint>
#include <memory>
#include <mutex>

#include "ESPressio_Platform.hpp"

namespace ESPressio {
namespace System {
namespace Synchronization {

constexpr uint32_t WaitForever = UINT32_MAX;

class ISignal {
public:
    virtual ~ISignal() = default;

    virtual PlatformResult Give() noexcept = 0;
    virtual PlatformResult GiveFromInterrupt() noexcept = 0;
    virtual PlatformResult Wait(uint32_t timeoutMilliseconds = WaitForever) noexcept = 0;
    virtual PlatformResult Reset() noexcept = 0;
};

class ISynchronizationProvider {
public:
    virtual ~ISynchronizationProvider() = default;

    virtual std::unique_ptr<ISignal> CreateBinarySignal(
        bool initiallySet = false
    ) = 0;
};

inline ISynchronizationProvider*& ProviderStorage() noexcept {
    static ISynchronizationProvider* provider = nullptr;
    return provider;
}

inline ISynchronizationProvider* Provider() noexcept {
    return ProviderStorage();
}

inline void SetProvider(ISynchronizationProvider* provider) noexcept {
    ProviderStorage() = provider;
}

inline void ResetProvider() noexcept {
    ProviderStorage() = nullptr;
}

class DeferredBinarySignal final : public ISignal {
private:
    bool _initiallySet = false;
    std::unique_ptr<ISignal> _signal;
    std::mutex _mutex;

    ISignal* Resolve() noexcept {
        if (_signal != nullptr) return _signal.get();

        std::lock_guard<std::mutex> lock(_mutex);
        if (_signal != nullptr) return _signal.get();

        auto* provider = Provider();
        if (provider == nullptr) return nullptr;

        _signal = provider->CreateBinarySignal(_initiallySet);
        return _signal.get();
    }

public:
    explicit DeferredBinarySignal(bool initiallySet) noexcept
        : _initiallySet(initiallySet) {}

    PlatformResult Give() noexcept override {
        auto* signal = Resolve();
        return signal != nullptr
            ? signal->Give()
            : PlatformResult::Failed(PlatformStatus::Unavailable);
    }

    PlatformResult GiveFromInterrupt() noexcept override {
        // Provider-backed objects may allocate during first materialization,
        // which is not safe from interrupt context. A signal intended for ISR
        // use must therefore have been resolved earlier from normal context.
        auto* signal = _signal.get();
        return signal != nullptr
            ? signal->GiveFromInterrupt()
            : PlatformResult::Failed(PlatformStatus::Unavailable);
    }

    PlatformResult Wait(uint32_t timeoutMilliseconds = WaitForever) noexcept override {
        auto* signal = Resolve();
        return signal != nullptr
            ? signal->Wait(timeoutMilliseconds)
            : PlatformResult::Failed(PlatformStatus::Unavailable);
    }

    PlatformResult Reset() noexcept override {
        auto* signal = Resolve();
        return signal != nullptr
            ? signal->Reset()
            : PlatformResult::Failed(PlatformStatus::Unavailable);
    }
};

inline std::unique_ptr<ISignal> CreateBinarySignal(bool initiallySet = false) {
    auto* provider = Provider();
    if (provider != nullptr) {
        auto signal = provider->CreateBinarySignal(initiallySet);
        if (signal != nullptr) return signal;
    }

    return std::make_unique<DeferredBinarySignal>(initiallySet);
}

}
}
}
