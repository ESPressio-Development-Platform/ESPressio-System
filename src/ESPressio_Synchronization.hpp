#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>

#include "ESPressio_Platform.hpp"

namespace ESPressio {
namespace System {
namespace Synchronization {

/// <summary>Timeout value representing an indefinite wait.</summary>
constexpr uint32_t WaitForever = UINT32_MAX;

/// <summary>Abstracts a binary synchronization signal that can be given, waited, and reset.</summary>
class ISignal {
public:
    virtual ~ISignal() = default;

    /// <summary>Sets the signal from normal execution context.</summary>
    virtual PlatformResult Give() noexcept = 0;
    /// <summary>Sets the signal from interrupt context using an interrupt-safe platform path.</summary>
    virtual PlatformResult GiveFromInterrupt() noexcept = 0;
    /// <summary>Waits until the signal is set or the timeout expires.</summary>
    virtual PlatformResult Wait(uint32_t timeoutMilliseconds = WaitForever) noexcept = 0;
    /// <summary>Clears the signal to its unset state.</summary>
    virtual PlatformResult Reset() noexcept = 0;
};

/// <summary>Creates platform-backed synchronization primitives.</summary>
class ISynchronizationProvider {
public:
    virtual ~ISynchronizationProvider() = default;

    /// <summary>Creates a binary signal with the requested initial state.</summary>
    virtual std::unique_ptr<ISignal> CreateBinarySignal(
        bool initiallySet = false
    ) = 0;
};

inline ISynchronizationProvider*& ProviderStorage() noexcept {
    static ISynchronizationProvider* provider = nullptr;
    return provider;
}

/// <summary>Gets the currently installed synchronization provider, or null when unavailable.</summary>
inline ISynchronizationProvider* Provider() noexcept {
    return ProviderStorage();
}

/// <summary>Installs the process-wide synchronization provider.</summary>
inline void SetProvider(ISynchronizationProvider* provider) noexcept {
    ProviderStorage() = provider;
}

/// <summary>Removes the currently installed synchronization provider.</summary>
inline void ResetProvider() noexcept {
    ProviderStorage() = nullptr;
}

/// <summary>Lazily resolves a binary signal when a synchronization provider becomes available.</summary>
/// <remarks>Interrupt-context giving is available only after the underlying signal has already been resolved.</remarks>
class DeferredBinarySignal final : public ISignal {
private:
    bool _initiallySet = false;
    std::unique_ptr<ISignal> _signal;
    std::atomic<ISignal*> _resolved{nullptr};
    std::mutex _mutex;

    ISignal* Resolve() noexcept {
        auto* resolved = _resolved.load(std::memory_order_acquire);
        if (resolved != nullptr) return resolved;

        std::lock_guard<std::mutex> lock(_mutex);
        resolved = _resolved.load(std::memory_order_relaxed);
        if (resolved != nullptr) return resolved;

        auto* provider = Provider();
        if (provider == nullptr) return nullptr;

        _signal = provider->CreateBinarySignal(_initiallySet);
        resolved = _signal.get();
        if (resolved != nullptr) {
            _resolved.store(resolved, std::memory_order_release);
        }
        return resolved;
    }

public:
    /// <summary>Creates a deferred binary signal with the requested initial state.</summary>
    explicit DeferredBinarySignal(bool initiallySet) noexcept
        : _initiallySet(initiallySet) {}

    /// <inheritdoc/>
    PlatformResult Give() noexcept override {
        auto* signal = Resolve();
        return signal != nullptr
            ? signal->Give()
            : PlatformResult::Failed(PlatformStatus::Unavailable);
    }

    /// <inheritdoc/>
    PlatformResult GiveFromInterrupt() noexcept override {
        auto* signal = _resolved.load(std::memory_order_acquire);
        return signal != nullptr
            ? signal->GiveFromInterrupt()
            : PlatformResult::Failed(PlatformStatus::Unavailable);
    }

    /// <inheritdoc/>
    PlatformResult Wait(uint32_t timeoutMilliseconds = WaitForever) noexcept override {
        auto* signal = Resolve();
        return signal != nullptr
            ? signal->Wait(timeoutMilliseconds)
            : PlatformResult::Failed(PlatformStatus::Unavailable);
    }

    /// <inheritdoc/>
    PlatformResult Reset() noexcept override {
        auto* signal = Resolve();
        return signal != nullptr
            ? signal->Reset()
            : PlatformResult::Failed(PlatformStatus::Unavailable);
    }
};

/// <summary>Creates a binary signal immediately when possible, otherwise returns a deferred signal.</summary>
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
