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
    PlatformStatus Status = PlatformStatus::Success;
    int32_t NativeCode = 0;

    constexpr explicit operator bool() const noexcept {
        return Status == PlatformStatus::Success;
    }

    static constexpr PlatformResult Succeeded() noexcept {
        return {};
    }

    static constexpr PlatformResult Failed(
        PlatformStatus status,
        int32_t nativeCode = 0
    ) noexcept {
        return {status, nativeCode};
    }
};

struct ProcessorAffinity {
    static constexpr int16_t AnyProcessor = -1;

    int16_t Processor = AnyProcessor;

    static constexpr ProcessorAffinity Any() noexcept {
        return {};
    }

    static constexpr ProcessorAffinity Specific(uint8_t processor) noexcept {
        return {static_cast<int16_t>(processor)};
    }

    constexpr bool IsSpecific() const noexcept {
        return Processor >= 0;
    }
};

}
}
