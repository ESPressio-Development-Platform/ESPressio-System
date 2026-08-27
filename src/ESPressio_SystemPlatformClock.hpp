#pragma once

#include <chrono>
#include <cstdint>
#include <memory>

#include "ESPressio_Platform.hpp"

namespace ESPressio {
namespace System {
namespace Clock {

class IMonotonicClock {
public:
    virtual ~IMonotonicClock() = default;
    virtual uint64_t NowNanoseconds() const noexcept = 0;
    virtual uint64_t ResolutionNanoseconds() const noexcept = 0;
    virtual bool IsInterruptSafe() const noexcept = 0;
};

class SteadyMonotonicClock final : public IMonotonicClock {
public:
    uint64_t NowNanoseconds() const noexcept override {
        const auto now = std::chrono::steady_clock::now().time_since_epoch();
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(now).count()
        );
    }
    uint64_t ResolutionNanoseconds() const noexcept override {
        using Period = std::chrono::steady_clock::period;
        const long double nanoseconds =
            (static_cast<long double>(Period::num) * 1000000000.0L) /
            static_cast<long double>(Period::den);
        return nanoseconds < 1.0L ? 1ULL : static_cast<uint64_t>(nanoseconds);
    }
    bool IsInterruptSafe() const noexcept override { return false; }
};

inline IMonotonicClock*& MonotonicClockStorage() noexcept {
    static SteadyMonotonicClock fallback;
    static IMonotonicClock* clock = &fallback;
    return clock;
}
inline IMonotonicClock& Monotonic() noexcept { return *MonotonicClockStorage(); }
inline void SetMonotonicClock(IMonotonicClock* clock) noexcept {
    if (clock != nullptr) MonotonicClockStorage() = clock;
}
inline void ResetMonotonicClock() noexcept {
    static SteadyMonotonicClock fallback;
    MonotonicClockStorage() = &fallback;
}

class IHighResolutionCounter {
public:
    virtual ~IHighResolutionCounter() = default;
    virtual PlatformResult Start() noexcept = 0;
    virtual PlatformResult Stop() noexcept = 0;
    virtual PlatformResult Reset() noexcept = 0;
    virtual PlatformResult Read(uint64_t& count) const noexcept = 0;
    virtual uint64_t ResolutionHz() const noexcept = 0;
    virtual bool IsAvailable() const noexcept = 0;
    virtual bool IsInterruptSafe() const noexcept = 0;
    virtual PlatformResult InitializationResult() const noexcept = 0;
};

class IHighResolutionCounterProvider {
public:
    virtual ~IHighResolutionCounterProvider() = default;
    virtual std::unique_ptr<IHighResolutionCounter> Create(uint64_t requestedResolutionHz) = 0;
};

inline IHighResolutionCounterProvider*& HighResolutionProviderStorage() noexcept {
    static IHighResolutionCounterProvider* provider = nullptr;
    return provider;
}
inline IHighResolutionCounterProvider* HighResolutionProvider() noexcept {
    return HighResolutionProviderStorage();
}
inline void SetHighResolutionCounterProvider(IHighResolutionCounterProvider* provider) noexcept {
    HighResolutionProviderStorage() = provider;
}
inline void ResetHighResolutionCounterProvider() noexcept {
    HighResolutionProviderStorage() = nullptr;
}
inline std::unique_ptr<IHighResolutionCounter> CreateHighResolutionCounter(uint64_t requestedResolutionHz) {
    auto* provider = HighResolutionProvider();
    return provider != nullptr ? provider->Create(requestedResolutionHz) : nullptr;
}

}
}
}
