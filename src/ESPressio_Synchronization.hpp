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
    virtual PlatformResult Give() noexcept = 0;
    virtual PlatformResult GiveFromInterrupt() noexcept = 0;
    virtual PlatformResult Wait(uint32_t timeoutMilliseconds = WaitForever) noexcept = 0;
    virtual PlatformResult Reset() noexcept = 0;
};

/// <summary>Abstracts an exclusive non-recursive mutex.</summary>
class IMutex {
public:
    virtual ~IMutex() = default;
    virtual void Lock() noexcept = 0;
    virtual bool TryLock() noexcept = 0;
    virtual void Unlock() noexcept = 0;
};

/// <summary>Abstracts an exclusive mutex that may be reacquired by its owning execution context.</summary>
class IRecursiveMutex {
public:
    virtual ~IRecursiveMutex() = default;
    virtual void Lock() noexcept = 0;
    virtual bool TryLock() noexcept = 0;
    virtual void Unlock() noexcept = 0;
};

/// <summary>Abstracts synchronization supporting shared readers and an exclusive writer.</summary>
class IReadWriteLock {
public:
    virtual ~IReadWriteLock() = default;
    virtual void Lock() noexcept = 0;
    virtual bool TryLock() noexcept = 0;
    virtual void Unlock() noexcept = 0;
    virtual void LockShared() noexcept = 0;
    virtual bool TryLockShared() noexcept = 0;
    virtual void UnlockShared() noexcept = 0;
};

/// <summary>Creates platform-backed synchronization primitives.</summary>
class ISynchronizationProvider {
public:
    virtual ~ISynchronizationProvider() = default;

    virtual std::unique_ptr<ISignal> CreateBinarySignal(bool initiallySet = false) = 0;
    virtual std::unique_ptr<IMutex> CreateMutex() { return {}; }
    virtual std::unique_ptr<IRecursiveMutex> CreateRecursiveMutex() { return {}; }
    virtual std::unique_ptr<IReadWriteLock> CreateReadWriteLock() { return {}; }
};

inline std::atomic<ISynchronizationProvider*>& ProviderStorage() noexcept {
    static std::atomic<ISynchronizationProvider*> provider{nullptr};
    return provider;
}

inline ISynchronizationProvider* Provider() noexcept {
    return ProviderStorage().load(std::memory_order_acquire);
}

inline void SetProvider(ISynchronizationProvider* provider) noexcept {
    ProviderStorage().store(provider, std::memory_order_release);
}

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
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
            try {
#endif
                auto* provider = Provider();
                if (provider != nullptr) _owned = provider->CreateMutex();
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
            } catch (...) {}
#endif
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
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
            try {
#endif
                auto* provider = Provider();
                if (provider != nullptr) _owned = provider->CreateRecursiveMutex();
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
            } catch (...) {}
#endif
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
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
            try {
#endif
                auto* provider = Provider();
                if (provider != nullptr) _owned = provider->CreateReadWriteLock();
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
            } catch (...) {}
#endif
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
    explicit DeferredBinarySignal(bool initiallySet) noexcept
        : _initiallySet(initiallySet) {}

    PlatformResult Give() noexcept override {
        auto* signal = Resolve();
        return signal != nullptr
            ? signal->Give()
            : PlatformResult::Failed(PlatformStatus::Unavailable);
    }

    PlatformResult GiveFromInterrupt() noexcept override {
        auto* signal = _resolved.load(std::memory_order_acquire);
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
