#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "ESPressio_Platform.hpp"

namespace ESPressio {
namespace System {
namespace Execution {

/// <summary>Opaque platform handle identifying an execution context.</summary>
using ExecutionHandle = uintptr_t;

/// <summary>Entry-point signature used to start a platform execution context.</summary>
using ExecutionEntry = void (*)(void*);

/// <summary>Sentinel value representing no valid execution context.</summary>
constexpr ExecutionHandle InvalidExecutionHandle = 0;

/// <summary>Configures the platform execution context created by an execution provider.</summary>
struct ExecutionConfiguration {
    /// <summary>Diagnostic name assigned to the execution context.</summary>
    const char* Name = "ESPressio";
    /// <summary>Requested stack capacity in bytes.</summary>
    std::size_t StackSizeBytes = 4096;
    /// <summary>Requested platform scheduling priority.</summary>
    uint32_t Priority = 1;
    /// <summary>Optional processor-affinity requirement.</summary>
    ProcessorAffinity Affinity = ProcessorAffinity::Any();
};

/// <summary>Contains the outcome and handle returned when an execution context is created.</summary>
struct ExecutionCreationResult {
    /// <summary>The platform operation result.</summary>
    PlatformResult Result;
    /// <summary>The created execution handle, or <c>InvalidExecutionHandle</c>.</summary>
    ExecutionHandle Handle;

    /// <summary>Creates an unavailable execution-creation result.</summary>
    ExecutionCreationResult() noexcept
        : Result(PlatformResult::Failed(PlatformStatus::Unavailable)),
          Handle(InvalidExecutionHandle) {}

    /// <summary>Creates an execution-creation result from an explicit result and handle.</summary>
    ExecutionCreationResult(
        PlatformResult result,
        ExecutionHandle handle
    ) noexcept
        : Result(result), Handle(handle) {}

    /// <summary>Indicates whether creation succeeded and produced a valid handle.</summary>
    explicit operator bool() const noexcept {
        return static_cast<bool>(Result) && Handle != InvalidExecutionHandle;
    }
};

/// <summary>Abstracts platform task/thread creation, lifecycle control, scheduling, and diagnostics.</summary>
class IExecutionProvider {
public:
    virtual ~IExecutionProvider() = default;

    /// <summary>Creates and starts a platform execution context.</summary>
    /// <param name="entry">Entry point invoked by the created context.</param>
    /// <param name="context">Opaque context passed to the entry point.</param>
    /// <param name="configuration">Execution configuration requested by the caller.</param>
    /// <returns>The creation result and resulting execution handle.</returns>
    virtual ExecutionCreationResult Create(
        ExecutionEntry entry,
        void* context,
        const ExecutionConfiguration& configuration
    ) = 0;

    /// <summary>Destroys the specified execution context.</summary>
    virtual PlatformResult Destroy(ExecutionHandle handle) = 0;
    /// <summary>Suspends the specified execution context.</summary>
    virtual PlatformResult Suspend(ExecutionHandle handle) = 0;
    /// <summary>Resumes the specified execution context.</summary>
    virtual PlatformResult Resume(ExecutionHandle handle) = 0;

    /// <summary>Gets the handle representing the currently executing context.</summary>
    virtual ExecutionHandle Current() const noexcept = 0;
    /// <summary>Gets the minimum observed free stack capacity for an execution context.</summary>
    virtual uint32_t MinimumFreeStackBytes(ExecutionHandle handle) const noexcept = 0;
    /// <summary>Gets the number of processors exposed by the platform.</summary>
    virtual uint32_t ProcessorCount() const noexcept = 0;

    /// <summary>Suspends the current execution context for the requested number of milliseconds.</summary>
    virtual void SleepMilliseconds(uint32_t milliseconds) = 0;
    /// <summary>Yields the current execution context to the platform scheduler.</summary>
    virtual void Yield() = 0;

    /// <summary>Indicates whether processor affinity is supported by this provider.</summary>
    virtual bool SupportsProcessorAffinity() const noexcept = 0;
};

/// <summary>Fallback execution provider that reports execution services as unavailable.</summary>
class NullExecutionProvider final : public IExecutionProvider {
public:
    ExecutionCreationResult Create(
        ExecutionEntry,
        void*,
        const ExecutionConfiguration&
    ) override {
        return ExecutionCreationResult(
            PlatformResult::Failed(PlatformStatus::Unavailable),
            InvalidExecutionHandle
        );
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

    uint32_t ProcessorCount() const noexcept override {
        return 1;
    }

    void SleepMilliseconds(uint32_t) override {}
    void Yield() override {}

    bool SupportsProcessorAffinity() const noexcept override {
        return false;
    }
};

inline NullExecutionProvider& FallbackProvider() noexcept {
    static NullExecutionProvider fallback;
    return fallback;
}

inline std::atomic<IExecutionProvider*>& ProviderStorage() noexcept {
    static std::atomic<IExecutionProvider*> provider{&FallbackProvider()};
    return provider;
}

/// <summary>Gets the active process-wide execution provider.</summary>
inline IExecutionProvider& Provider() noexcept {
    auto* provider = ProviderStorage().load(std::memory_order_acquire);
    return provider != nullptr ? *provider : FallbackProvider();
}

/// <summary>Installs a non-null process-wide execution provider.</summary>
/// <param name="provider">Provider to install. Null values are ignored.</param>
inline void SetProvider(IExecutionProvider* provider) noexcept {
    if (provider != nullptr) {
        ProviderStorage().store(provider, std::memory_order_release);
    }
}

/// <summary>Restores the fallback execution provider.</summary>
inline void ResetProvider() noexcept {
    ProviderStorage().store(&FallbackProvider(), std::memory_order_release);
}

}
}
}
