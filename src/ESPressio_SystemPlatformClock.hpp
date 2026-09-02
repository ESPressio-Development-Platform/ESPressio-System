#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>

#include "ESPressio_Platform.hpp"

namespace ESPressio {
namespace System {
namespace Clock {

/// <summary>Provides a monotonically increasing platform time source expressed in nanoseconds.</summary>
class IMonotonicClock {
public:
    virtual ~IMonotonicClock() = default;

    /// <summary>Gets the current monotonic time in nanoseconds.</summary>
    virtual uint64_t NowNanoseconds() const noexcept = 0;
    /// <summary>Gets the effective clock resolution in nanoseconds.</summary>
    virtual uint64_t ResolutionNanoseconds() const noexcept = 0;
    /// <summary>Indicates whether the clock can be safely read from interrupt context.</summary>
    virtual bool IsInterruptSafe() const noexcept = 0;
};

/// <summary>Portable monotonic clock implementation backed by <c>std::chrono::steady_clock</c>.</summary>
class SteadyMonotonicClock final : public IMonotonicClock {
public:
    /// <inheritdoc/>
    uint64_t NowNanoseconds() const noexcept override {
        const auto now = std::chrono::steady_clock::now().time_since_epoch();
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(now).count()
        );
    }
    /// <inheritdoc/>
    uint64_t ResolutionNanoseconds() const noexcept override {
        using Period = std::chrono::steady_clock::period;
        const long double nanoseconds =
            (static_cast<long double>(Period::num) * 1000000000.0L) /
            static_cast<long double>(Period::den);
        return nanoseconds < 1.0L ? 1ULL : static_cast<uint64_t>(nanoseconds);
    }
    /// <inheritdoc/>
    bool IsInterruptSafe() const noexcept override { return false; }
};

inline SteadyMonotonicClock& FallbackMonotonicClock() noexcept {
    static SteadyMonotonicClock fallback;
    return fallback;
}

inline std::atomic<IMonotonicClock*>& MonotonicClockStorage() noexcept {
    static std::atomic<IMonotonicClock*> clock{&FallbackMonotonicClock()};
    return clock;
}

/// <summary>Gets the active process-wide monotonic clock.</summary>
inline IMonotonicClock& Monotonic() noexcept {
    auto* clock = MonotonicClockStorage().load(std::memory_order_acquire);
    return clock != nullptr ? *clock : FallbackMonotonicClock();
}

/// <summary>Installs a non-null process-wide monotonic clock.</summary>
inline void SetMonotonicClock(IMonotonicClock* clock) noexcept {
    if (clock != nullptr) {
        MonotonicClockStorage().store(clock, std::memory_order_release);
    }
}

/// <summary>Restores the portable steady-clock fallback.</summary>
inline void ResetMonotonicClock() noexcept {
    MonotonicClockStorage().store(
        &FallbackMonotonicClock(),
        std::memory_order_release
    );
}

/// <summary>Represents a controllable high-resolution platform counter.</summary>
class IHighResolutionCounter {
public:
    virtual ~IHighResolutionCounter() = default;

    /// <summary>Starts counter progression.</summary>
    virtual PlatformResult Start() noexcept = 0;
    /// <summary>Stops counter progression.</summary>
    virtual PlatformResult Stop() noexcept = 0;
    /// <summary>Resets the counter value to its platform-defined origin.</summary>
    virtual PlatformResult Reset() noexcept = 0;
    /// <summary>Reads the current raw counter value.</summary>
    virtual PlatformResult Read(uint64_t& count) const noexcept = 0;
    /// <summary>Gets the effective counter frequency in hertz.</summary>
    virtual uint64_t ResolutionHz() const noexcept = 0;
    /// <summary>Indicates whether the counter is available for use.</summary>
    virtual bool IsAvailable() const noexcept = 0;
    /// <summary>Indicates whether the counter can be safely read from interrupt context.</summary>
    virtual bool IsInterruptSafe() const noexcept = 0;
    /// <summary>Gets the result produced while initializing the counter.</summary>
    virtual PlatformResult InitializationResult() const noexcept = 0;
};

/// <summary>Creates high-resolution counters using the active platform implementation.</summary>
class IHighResolutionCounterProvider {
public:
    virtual ~IHighResolutionCounterProvider() = default;

    /// <summary>Creates a counter targeting the requested frequency.</summary>
    /// <param name="requestedResolutionHz">Desired counter resolution in hertz.</param>
    virtual std::unique_ptr<IHighResolutionCounter> Create(uint64_t requestedResolutionHz) = 0;
};

inline std::atomic<IHighResolutionCounterProvider*>& HighResolutionProviderStorage() noexcept {
    static std::atomic<IHighResolutionCounterProvider*> provider{nullptr};
    return provider;
}

/// <summary>Gets the currently installed high-resolution counter provider.</summary>
inline IHighResolutionCounterProvider* HighResolutionProvider() noexcept {
    return HighResolutionProviderStorage().load(std::memory_order_acquire);
}

/// <summary>Installs the process-wide high-resolution counter provider.</summary>
inline void SetHighResolutionCounterProvider(IHighResolutionCounterProvider* provider) noexcept {
    HighResolutionProviderStorage().store(provider, std::memory_order_release);
}

/// <summary>Removes the currently installed high-resolution counter provider.</summary>
inline void ResetHighResolutionCounterProvider() noexcept {
    HighResolutionProviderStorage().store(nullptr, std::memory_order_release);
}

/// <summary>Creates a high-resolution counter through the active provider.</summary>
/// <param name="requestedResolutionHz">Desired counter resolution in hertz.</param>
/// <returns>The created counter, or null when no provider is installed.</returns>
inline std::unique_ptr<IHighResolutionCounter> CreateHighResolutionCounter(uint64_t requestedResolutionHz) {
    auto* provider = HighResolutionProvider();
    return provider != nullptr ? provider->Create(requestedResolutionHz) : nullptr;
}

}
}
}