#pragma once
#include <atomic>
#include "ESPressio_DeviceRuntimeIdentity.hpp"

namespace ESPressio::System {
/// <summary>Single process identity authority; no instances, reset or replacement are exposed.</summary>
class RuntimeIdentity final {
    RuntimeIdentity() = delete;
public:
    /// <summary>Install failure never replaces an existing identity or partially publishes a candidate.</summary>
    enum class InstallationStatus { Success, InvalidIdentity, AlreadyInstalled };
private:
    // Constant-initialized process-lifetime storage. Only the winning installer writes Identity,
    // and readers access it only after an acquire observes the release publication of Ready.
    inline static DeviceRuntimeIdentity Identity{};
    inline static std::atomic<unsigned char> State{0}; // Empty=0, Installing=1, Ready=2
public:
    /// <summary>Installs once after Persistence has durably committed this candidate.</summary>
    /// <remarks>Bootstrap must not expose/use a candidate before the durable commit. System
    /// intentionally cannot inspect persistence. Concurrent installers have one winner; all
    /// others return AlreadyInstalled and cannot replace it. Installation cannot be reset.</remarks>
    static InstallationStatus Install(const DeviceRuntimeIdentity& identity) noexcept {
        if (!identity) return InstallationStatus::InvalidIdentity;
        unsigned char empty = 0;
        if (!State.compare_exchange_strong(empty, 1, std::memory_order_acq_rel))
            return InstallationStatus::AlreadyInstalled;
        Identity = identity;
        State.store(2, std::memory_order_release);
        return InstallationStatus::Success;
    }
    /// <summary>Gets the immutable process-lifetime identity, or null before publication.</summary>
    /// <remarks>Bounded and allocation-free; never waits for bootstrap or calls a provider.</remarks>
    static const DeviceRuntimeIdentity* TryGet() noexcept {
        return State.load(std::memory_order_acquire) == 2 ? &Identity : nullptr;
    }
    /// <summary>Reports whether a complete installed identity is available.</summary>
    static bool IsInstalled() noexcept { return TryGet() != nullptr; }
    /// <summary>Copies an installed identity. Unavailable leaves the caller's output untouched.</summary>
    static bool TryRead(DeviceRuntimeIdentity& output) noexcept {
        const auto* identity = TryGet();
        if (!identity) return false;
        output = *identity;
        return true;
    }
};
} // namespace ESPressio::System
