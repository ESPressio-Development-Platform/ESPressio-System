#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <shared_mutex>

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

/// <summary>Abstracts an exclusive non-recursive mutex.</summary>
class IMutex {
public:
    virtual ~IMutex() = default;
    /// <summary>Blocks until exclusive ownership is acquired.</summary>
    virtual void Lock() noexcept = 0;
    /// <summary>Attempts to acquire exclusive ownership without blocking.</summary>
    virtual bool TryLock() noexcept = 0;
    /// <summary>Releases exclusive ownership.</summary>
    virtual void Unlock() noexcept = 0;
};

/// <summary>Abstracts an exclusive mutex that may be reacquired by its owning execution context.</summary>
class IRecursiveMutex {
public:
    virtual ~IRecursiveMutex() = default;
    /// <summary>Blocks until recursive exclusive ownership is acquired.</summary>
    virtual void Lock() noexcept = 0;
    /// <summary>Attempts to acquire recursive exclusive ownership without blocking.</summary>
    virtual bool TryLock() noexcept = 0;
    /// <summary>Releases one recursive ownership level.</summary>
    virtual void Unlock() noexcept = 0;
};

/// <summary>Abstracts synchronization supporting shared readers and an exclusive writer.</summary>
class IReadWriteLock {
public:
    virtual ~IReadWriteLock() = default;
    /// <summary>Blocks until exclusive ownership is acquired.</summary>
    virtual void Lock() noexcept = 0;
    /// <summary>Attempts to acquire exclusive ownership without blocking.</summary>
    virtual bool TryLock() noexcept = 0;
    /// <summary>Releases exclusive ownership.</summary>
    virtual void Unlock() noexcept = 0;
    /// <summary>Blocks until shared ownership is acquired.</summary>
    virtual void LockShared() noexcept = 0;
    /// <summary>Attempts to acquire shared ownership without blocking.</summary>
    virtual bool TryLockShared() noexcept = 0;
    /// <summary>Releases shared ownership.</summary>
    virtual void UnlockShared() noexcept = 0;
};

/// <summary>Creates platform-backed synchronization primitives.</summary>
class ISynchronizationProvider {
public:
    virtual ~ISynchronizationProvider() = default;

    /// <summary>Creates a binary signal with the requested initial state.</summary>
    virtual std::unique_ptr<ISignal> CreateBinarySignal(
        bool initiallySet = false
    ) = 0;

    /// <summary>Creates an exclusive non-recursive mutex.</summary>
    virtual std::unique_ptr<IMutex> CreateMutex() { return {}; }

    /// <summary>Creates an exclusive recursive mutex.</summary>
    virtual std::unique_ptr<IRecursiveMutex> CreateRecursiveMutex() { return {}; }

    /// <summary>Creates a read/write synchronization primitive.</summary>
    virtual std::unique_ptr<IReadWriteLock> CreateReadWriteLock() { return {}; }
};

inline std::atomic<ISynchronizationProvider*>& ProviderStorage() noexcept {
    static std::atomic<ISynchronizationProvider*> provider{nullptr};
    return provider;
}

/// <summary>Gets the currently installed synchronization provider, or null when unavailable.</summary>
inline ISynchronizationProvider* Provider() noexcept {
    return ProviderStorage().load(std::memory_order_acquire);
}

/// <summary>Installs the process-wide synchronization provider.</summary>
inline void SetProvider(ISynchronizationProvider* provider) noexcept {
    ProviderStorage().store(provider, std::memory_order_release);
}

/// <summary>Removes the currently installed synchronization provider.</summary>
inline void ResetProvider() noexcept {
    ProviderStorage().store(nullptr, std::memory_order_release);
}

namespace Detail {

class StandardMutex final : public IMutex {
    std::mutex _mutex;
public:
    void Lock() noexcept override { _mutex.lock(); }
    bool TryLock() noexcept override { return _mutex.try_lock(); }
    void Unlock() noexcept override { _mutex.unlock(); }
};

class StandardRecursiveMutex final : public IRecursiveMutex {
    std::recursive_mutex _mutex;
public:
    void Lock() noexcept override { _mutex.lock(); }
    bool TryLock() noexcept override { return _mutex.try_lock(); }
    void Unlock() noexcept override { _mutex.unlock(); }
};

class StandardReadWriteLock final : public IReadWriteLock {
    std::shared_mutex _mutex;
public:
    void Lock() noexcept override { _mutex.lock(); }
    bool TryLock() noexcept override { return _mutex.try_lock(); }
    void Unlock() noexcept override { _mutex.unlock(); }
    void LockShared() noexcept override { _mutex.lock_shared(); }
    bool TryLockShared() noexcept override { return _mutex.try_lock_shared(); }
    void UnlockShared() noexcept override { _mutex.unlock_shared(); }
};

/// <summary>Serializes lazy provider resolution without allocating another platform primitive.</summary>
class ResolutionGuard final {
    std::atomic_flag& _flag;
public:
    explicit ResolutionGuard(std::atomic_flag& flag) noexcept : _flag(flag) {
        while (_flag.test_and_set(std::memory_order_acquire)) {}
    }
    ~ResolutionGuard() { _flag.clear(std::memory_order_release); }
};

} // namespace Detail

/// <summary>Provider-aware non-recursive mutex with a standard C++ fallback.</summary>
/// <remarks>The platform primitive is created lazily on first use. If no provider is available at that point, the embedded portable fallback is selected permanently for this instance, preserving safe use by process-lifetime objects constructed before platform installation.</remarks>
class Mutex final {
    std::unique_ptr<IMutex> _owned;
    Detail::StandardMutex _fallback;
    std::atomic<IMutex*> _resolved{nullptr};
    std::atomic_flag _resolutionGuard = ATOMIC_FLAG_INIT;

    IMutex& Resolve() noexcept {
        auto* resolved = _resolved.load(std::memory_order_acquire);
        if (resolved != nullptr) return *resolved;
        Detail::ResolutionGuard guard(_resolutionGuard);
        resolved = _resolved.load(std::memory_order_relaxed);
        if (resolved == nullptr) {
            try {
                auto* provider = Provider();
                if (provider != nullptr) _owned = provider->CreateMutex();
            } catch (...) {}
            resolved = _owned ? _owned.get() : static_cast<IMutex*>(&_fallback);
            _resolved.store(resolved, std::memory_order_release);
        }
        return *resolved;
    }

public:
    Mutex() = default;
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    void lock() noexcept { Resolve().Lock(); }
    bool try_lock() noexcept { return Resolve().TryLock(); }
    void unlock() noexcept { Resolve().Unlock(); }
};

/// <summary>Provider-aware recursive mutex with a standard C++ fallback.</summary>
class RecursiveMutex final {
    std::unique_ptr<IRecursiveMutex> _owned;
    Detail::StandardRecursiveMutex _fallback;
    std::atomic<IRecursiveMutex*> _resolved{nullptr};
    std::atomic_flag _resolutionGuard = ATOMIC_FLAG_INIT;

    IRecursiveMutex& Resolve() noexcept {
        auto* resolved = _resolved.load(std::memory_order_acquire);
        if (resolved != nullptr) return *resolved;
        Detail::ResolutionGuard guard(_resolutionGuard);
        resolved = _resolved.load(std::memory_order_relaxed);
        if (resolved == nullptr) {
            try {
                auto* provider = Provider();
                if (provider != nullptr) _owned = provider->CreateRecursiveMutex();
            } catch (...) {}
            resolved = _owned ? _owned.get() : static_cast<IRecursiveMutex*>(&_fallback);
            _resolved.store(resolved, std::memory_order_release);
        }
        return *resolved;
    }

public:
    RecursiveMutex() = default;
    RecursiveMutex(const RecursiveMutex&) = delete;
    RecursiveMutex& operator=(const RecursiveMutex&) = delete;
    void lock() noexcept { Resolve().Lock(); }
    bool try_lock() noexcept { return Resolve().TryLock(); }
    void unlock() noexcept { Resolve().Unlock(); }
};

/// <summary>Provider-aware read/write lock with a standard C++ fallback.</summary>
class ReadWriteLock final {
    std::unique_ptr<IReadWriteLock> _owned;
    Detail::StandardReadWriteLock _fallback;
    std::atomic<IReadWriteLock*> _resolved{nullptr};
    std::atomic_flag _resolutionGuard = ATOMIC_FLAG_INIT;

    IReadWriteLock& Resolve() noexcept {
        auto* resolved = _resolved.load(std::memory_order_acquire);
        if (resolved != nullptr) return *resolved;
        Detail::ResolutionGuard guard(_resolutionGuard);
        resolved = _resolved.load(std::memory_order_relaxed);
        if (resolved == nullptr) {
            try {
                auto* provider = Provider();
                if (provider != nullptr) _owned = provider->CreateReadWriteLock();
            } catch (...) {}
            resolved = _owned ? _owned.get() : static_cast<IReadWriteLock*>(&_fallback);
            _resolved.store(resolved, std::memory_order_release);
        }
        return *resolved;
    }

public:
    ReadWriteLock() = default;
    ReadWriteLock(const ReadWriteLock&) = delete;
    ReadWriteLock& operator=(const ReadWriteLock&) = delete;
    void lock() noexcept { Resolve().Lock(); }
    bool try_lock() noexcept { return Resolve().TryLock(); }
    void unlock() noexcept { Resolve().Unlock(); }
    void lock_shared() noexcept { Resolve().LockShared(); }
    bool try_lock_shared() noexcept { return Resolve().TryLockShared(); }
    void unlock_shared() noexcept { Resolve().UnlockShared(); }
};

/// <summary>Lazily resolves a binary signal when a synchronization provider becomes available.</summary>
/// <remarks>Interrupt-context giving is available only after the underlying signal has already been resolved.</remarks>
class DeferredBinarySignal final : public ISignal {
private:
    bool _initiallySet = false;
    std::unique_ptr<ISignal> _signal;
    std::atomic<ISignal*> _resolved{nullptr};
    Mutex _mutex;

    ISignal* Resolve() noexcept {
        auto* resolved = _resolved.load(std::memory_order_acquire);
        if (resolved != nullptr) return resolved;

        std::lock_guard<Mutex> lock(_mutex);
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