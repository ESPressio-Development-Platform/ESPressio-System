#pragma once

#include <cstdint>

namespace ESPressio {
namespace System {

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

struct PlatformResult {
    PlatformStatus Status;
    int32_t NativeCode;

    constexpr PlatformResult() noexcept
        : Status(PlatformStatus::Success), NativeCode(0) {}

    constexpr PlatformResult(
        PlatformStatus status,
        int32_t nativeCode = 0
    ) noexcept
        : Status(status), NativeCode(nativeCode) {}

    constexpr explicit operator bool() const noexcept {
        return Status == PlatformStatus::Success;
    }

    static constexpr PlatformResult Succeeded() noexcept {
        return PlatformResult();
    }

    static constexpr PlatformResult Failed(
        PlatformStatus status,
        int32_t nativeCode = 0
    ) noexcept {
        return PlatformResult(status, nativeCode);
    }
};

struct ProcessorAffinity {
    static constexpr int16_t AnyProcessor = -1;

    int16_t Processor;

    constexpr ProcessorAffinity() noexcept
        : Processor(AnyProcessor) {}

    constexpr explicit ProcessorAffinity(int16_t processor) noexcept
        : Processor(processor) {}

    static constexpr ProcessorAffinity Any() noexcept {
        return ProcessorAffinity();
    }

    static constexpr ProcessorAffinity Specific(uint8_t processor) noexcept {
        return ProcessorAffinity(static_cast<int16_t>(processor));
    }

    constexpr bool IsSpecific() const noexcept {
        return Processor >= 0;
    }
};

}
}
