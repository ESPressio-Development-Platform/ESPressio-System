#pragma once

#include <cstdint>
#include <memory>
#include <utility>

#include "ESPressio_Platform.hpp"

namespace ESPressio {
namespace System {
namespace GPIO {

/// <summary>Portable GPIO pin identifier.</summary>
using Pin = uint16_t;

/// <summary>Logical GPIO signal level.</summary>
enum class State : uint8_t {
    Low = 0,
    High = 1
};

/// <summary>Configures the electrical direction of a GPIO pin.</summary>
enum class Direction : uint8_t {
    Input,
    Output,
    OpenDrain
};

/// <summary>Configures an optional GPIO pull resistor.</summary>
enum class Pull : uint8_t {
    None,
    Up,
    Down,
    UpDown
};

/// <summary>Specifies the signal condition that triggers a GPIO interrupt.</summary>
enum class InterruptTrigger : uint8_t {
    RisingEdge,
    FallingEdge,
    AnyEdge,
    LowLevel,
    HighLevel
};

/// <summary>Describes the direction, pull configuration, and initial level for a GPIO pin.</summary>
struct PinConfiguration {
    /// <summary>Requested GPIO direction.</summary>
    Direction DirectionMode = Direction::Input;
    /// <summary>Requested internal pull configuration.</summary>
    Pull PullMode = Pull::None;
    /// <summary>Initial output state when applicable.</summary>
    State InitialState = State::Low;
};

/// <summary>Describes the trigger, affinity, and initial enabled state of a GPIO interrupt.</summary>
struct InterruptConfiguration {
    /// <summary>Signal condition that causes the interrupt.</summary>
    InterruptTrigger Trigger = InterruptTrigger::AnyEdge;
    /// <summary>Optional processor affinity for interrupt handling.</summary>
    ProcessorAffinity Affinity = ProcessorAffinity::Any();
    /// <summary>Whether the interrupt should be enabled immediately after creation.</summary>
    bool StartEnabled = true;
};

/// <summary>Callback signature invoked when a configured GPIO interrupt fires.</summary>
using InterruptCallback = void (*)(void* context);

/// <summary>Represents a configured GPIO interrupt and exposes its runtime lifecycle.</summary>
class IInterrupt {
public:
    virtual ~IInterrupt() = default;

    /// <summary>Gets the GPIO pin associated with this interrupt.</summary>
    virtual Pin GetPin() const noexcept = 0;
    /// <summary>Gets the processor affinity assigned to this interrupt.</summary>
    virtual ProcessorAffinity GetAffinity() const noexcept = 0;
    /// <summary>Indicates whether this interrupt is currently enabled.</summary>
    virtual bool IsEnabled() const noexcept = 0;

    /// <summary>Enables interrupt delivery.</summary>
    virtual PlatformResult Enable() noexcept = 0;
    /// <summary>Disables interrupt delivery.</summary>
    virtual PlatformResult Disable() noexcept = 0;
};

/// <summary>Owning handle for a configured GPIO interrupt.</summary>
using InterruptHandle = std::unique_ptr<IInterrupt>;

/// <summary>Contains the outcome and optional handle returned when creating a GPIO interrupt.</summary>
struct InterruptCreationResult {
    /// <summary>The platform operation result.</summary>
    PlatformResult Result = PlatformResult::Failed(PlatformStatus::Unavailable);
    /// <summary>The created interrupt handle when creation succeeds.</summary>
    InterruptHandle Handle;

    InterruptCreationResult() = default;

    /// <summary>Creates an interrupt-creation result from an explicit result and handle.</summary>
    InterruptCreationResult(
        PlatformResult result,
        InterruptHandle handle = nullptr
    )
        : Result(result),
          Handle(std::move(handle)) {}

    InterruptCreationResult(InterruptCreationResult&&) noexcept = default;
    InterruptCreationResult& operator=(InterruptCreationResult&&) noexcept = default;
    InterruptCreationResult(const InterruptCreationResult&) = delete;
    InterruptCreationResult& operator=(const InterruptCreationResult&) = delete;

    /// <summary>Indicates whether creation succeeded and produced an interrupt handle.</summary>
    explicit operator bool() const noexcept {
        return static_cast<bool>(Result) && Handle != nullptr;
    }
};

/// <summary>Abstracts platform GPIO configuration, I/O, and interrupt creation.</summary>
class IController {
public:
    virtual ~IController() = default;

    /// <summary>Applies the requested configuration to a GPIO pin.</summary>
    virtual PlatformResult Configure(
        Pin pin,
        const PinConfiguration& configuration
    ) noexcept = 0;

    /// <summary>Writes a logical state to a GPIO pin.</summary>
    virtual PlatformResult Write(Pin pin, State state) noexcept = 0;
    /// <summary>Reads the current logical state of a GPIO pin.</summary>
    virtual PlatformResult Read(Pin pin, State& state) const noexcept = 0;

    /// <summary>Creates an interrupt for the specified GPIO pin.</summary>
    /// <param name="pin">GPIO pin to observe.</param>
    /// <param name="configuration">Interrupt trigger and affinity configuration.</param>
    /// <param name="callback">Callback invoked when the interrupt fires.</param>
    /// <param name="context">Optional context supplied to the callback.</param>
    virtual InterruptCreationResult CreateInterrupt(
        Pin pin,
        const InterruptConfiguration& configuration,
        InterruptCallback callback,
        void* context = nullptr
    ) = 0;

    /// <summary>Indicates whether GPIO interrupts are supported by this controller.</summary>
    virtual bool SupportsInterrupts() const noexcept = 0;
    /// <summary>Indicates whether GPIO interrupt processor affinity is supported.</summary>
    virtual bool SupportsInterruptAffinity() const noexcept = 0;
};

inline IController*& ControllerStorage() noexcept {
    static IController* controller = nullptr;
    return controller;
}

/// <summary>Gets the currently installed GPIO controller, or null when none is configured.</summary>
inline IController* Controller() noexcept {
    return ControllerStorage();
}

/// <summary>Installs the process-wide GPIO controller.</summary>
inline void SetController(IController* controller) noexcept {
    ControllerStorage() = controller;
}

/// <summary>Removes the currently installed GPIO controller.</summary>
inline void ResetController() noexcept {
    ControllerStorage() = nullptr;
}

}
}
}
