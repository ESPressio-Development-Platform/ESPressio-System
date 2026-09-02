#pragma once

#include <cstdint>

namespace ESPressio {
namespace System {

/// <summary>Identifies the portable outcome category returned by platform abstractions.</summary>
enum class PlatformStatus : uint8_t {
    Success,
    Unsupported,
    Unavailable,
    InvalidArgument,
    Busy,
    Conflict,
    OutOfMemory,
    Timeout,
    HardwareFailure,
    Unknown
};

/// <summary>Represents the portable status and optional native platform code produced by an operation.</summary>
struct PlatformResult {
    /// <summary>The portable status category.</summary>
    PlatformStatus Status;

    /// <summary>An optional platform-specific status or error code.</summary>
    int32_t NativeCode;

    /// <summary>Creates a successful result.</summary>
    constexpr PlatformResult() noexcept
        : Status(PlatformStatus::Success), NativeCode(0) {}

    /// <summary>Creates a result with an explicit portable status and optional native code.</summary>
    /// <param name="status">The portable status category.</param>
    /// <param name="nativeCode">An optional platform-specific status or error code.</param>
    constexpr PlatformResult(
        PlatformStatus status,
        int32_t nativeCode = 0
    ) noexcept
        : Status(status), NativeCode(nativeCode) {}

    /// <summary>Indicates whether the result represents success.</summary>
    constexpr explicit operator bool() const noexcept {
        return Status == PlatformStatus::Success;
    }

    /// <summary>Creates a successful platform result.</summary>
    static constexpr PlatformResult Succeeded() noexcept {
        return PlatformResult();
    }

    /// <summary>Creates a failed platform result.</summary>
    /// <param name="status">The failure status category.</param>
    /// <param name="nativeCode">An optional platform-specific error code.</param>
    static constexpr PlatformResult Failed(
        PlatformStatus status,
        int32_t nativeCode = 0
    ) noexcept {
        return PlatformResult(status, nativeCode);
    }
};

/// <summary>Describes optional processor affinity for platform execution resources.</summary>
struct ProcessorAffinity {
    /// <summary>Sentinel value indicating that no specific processor is requested.</summary>
    static constexpr int16_t AnyProcessor = -1;

    /// <summary>The requested processor index, or <c>AnyProcessor</c>.</summary>
    int16_t Processor;

    /// <summary>Creates an affinity that permits execution on any processor.</summary>
    constexpr ProcessorAffinity() noexcept
        : Processor(AnyProcessor) {}

    /// <summary>Creates an affinity targeting the supplied processor value.</summary>
    /// <param name="processor">The processor index, or <c>AnyProcessor</c>.</param>
    constexpr explicit ProcessorAffinity(int16_t processor) noexcept
        : Processor(processor) {}

    /// <summary>Creates an affinity that permits execution on any processor.</summary>
    static constexpr ProcessorAffinity Any() noexcept {
        return ProcessorAffinity();
    }

    /// <summary>Creates an affinity targeting a specific processor.</summary>
    /// <param name="processor">The zero-based processor index.</param>
    static constexpr ProcessorAffinity Specific(uint8_t processor) noexcept {
        return ProcessorAffinity(static_cast<int16_t>(processor));
    }

    /// <summary>Indicates whether this affinity targets one specific processor.</summary>
    constexpr bool IsSpecific() const noexcept {
        return Processor >= 0;
    }
};

}
}
