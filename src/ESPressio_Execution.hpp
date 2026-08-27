#pragma once

#include <cstddef>
#include <cstdint>

#include "ESPressio_Platform.hpp"

namespace ESPressio {
namespace System {
namespace Execution {

using ExecutionHandle = uintptr_t;
using ExecutionEntry = void (*)(void*);

constexpr ExecutionHandle InvalidExecutionHandle = 0;

struct ExecutionConfiguration {
    const char* Name = "ESPressio";
    std::size_t StackSizeBytes = 4096;
    uint32_t Priority = 1;
    ProcessorAffinity Affinity = ProcessorAffinity::Any();
};

struct ExecutionCreationResult {
    PlatformResult Result = PlatformResult::Failed(PlatformStatus::Unavailable);
    ExecutionHandle Handle = InvalidExecutionHandle;

    explicit operator bool() const noexcept {
        return static_cast<bool>(Result) && Handle != InvalidExecutionHandle;
    }
};

class IExecutionProvider {
public:
    virtual ~IExecutionProvider() = default;

    virtual ExecutionCreationResult Create(
        ExecutionEntry entry,
        void* context,
        const ExecutionConfiguration& configuration
    ) = 0;

    virtual PlatformResult Destroy(ExecutionHandle handle) = 0;
    virtual PlatformResult Suspend(ExecutionHandle handle) = 0;
    virtual PlatformResult Resume(ExecutionHandle handle) = 0;

    virtual ExecutionHandle Current() const noexcept = 0;
    virtual uint32_t MinimumFreeStackBytes(ExecutionHandle handle) const noexcept = 0;

    virtual void SleepMilliseconds(uint32_t milliseconds) = 0;
    virtual void Yield() = 0;

    virtual bool SupportsProcessorAffinity() const noexcept = 0;
};

class NullExecutionProvider final : public IExecutionProvider {
public:
    ExecutionCreationResult Create(
        ExecutionEntry,
        void*,
        const ExecutionConfiguration&
    ) override {
        return {
            PlatformResult::Failed(PlatformStatus::Unavailable),
            InvalidExecutionHandle
        };
    }

    PlatformResult Destroy(ExecutionHandle) override {
        return PlatformResult::Failed(PlatformStatus::Unavailable);
    }

    PlatformResult Suspend(ExecutionHandle) override {
        return PlatformResult::Failed(PlatformStatus::Unavailable);
    }

    PlatformResult Resume(ExecutionHandle) override {
        return PlatformResult::Failed(PlatformStatus::Unavailable);
    }

    ExecutionHandle Current() const noexcept override {
        return InvalidExecutionHandle;
    }

    uint32_t MinimumFreeStackBytes(ExecutionHandle) const noexcept override {
        return 0;
    }

    void SleepMilliseconds(uint32_t) override {}
    void Yield() override {}

    bool SupportsProcessorAffinity() const noexcept override {
        return false;
    }
};

inline IExecutionProvider*& ProviderStorage() noexcept {
    static NullExecutionProvider fallback;
    static IExecutionProvider* provider = &fallback;
    return provider;
}

inline IExecutionProvider& Provider() noexcept {
    return *ProviderStorage();
}

inline void SetProvider(IExecutionProvider* provider) noexcept {
    if (provider != nullptr) {
        ProviderStorage() = provider;
    }
}

inline void ResetProvider() noexcept {
    static NullExecutionProvider fallback;
    ProviderStorage() = &fallback;
}

}
}
}
