#pragma once
#include "ESPressio_DeviceIdentifier.hpp"
#include "ESPressio_RuntimeIncarnationId.hpp"

namespace ESPressio::System {
/// <summary>Device identity and durable incarnation for one actual System/process runtime.</summary>
/// <remarks>Transport address, service restart count and authentication are separate concepts.</remarks>
struct DeviceRuntimeIdentity final {
    System::DeviceIdentifier Device{};
    RuntimeIncarnationId Incarnation{};
    constexpr explicit operator bool() const noexcept { return bool(Device) && bool(Incarnation); }
    constexpr bool operator==(const DeviceRuntimeIdentity& other) const noexcept {
        return Device == other.Device && Incarnation == other.Incarnation;
    }
    constexpr bool operator!=(const DeviceRuntimeIdentity& other) const noexcept { return !(*this == other); }
};
static_assert(sizeof(DeviceRuntimeIdentity) == 20, "Runtime identity contains only the 16+4 byte semantic values");
} // namespace ESPressio::System
