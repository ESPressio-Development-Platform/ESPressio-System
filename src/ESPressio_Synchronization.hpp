#pragma once

#include <cstdint>
#include <memory>

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

inline std::unique_ptr<ISignal> CreateBinarySignal(bool initiallySet = false) {
    auto* provider = Provider();
    return provider != nullptr
        ? provider->CreateBinarySignal(initiallySet)
        : nullptr;
}

}
}
}
