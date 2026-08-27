#pragma once

#include <cstdint>
#include <memory>

#include "ESPressio_Platform.hpp"

namespace ESPressio {
namespace System {
namespace GPIO {

using Pin = uint16_t;

enum class State : uint8_t {
    Low = 0,
    High = 1
};

enum class Direction : uint8_t {
    Input,
    Output,
    OpenDrain
};

enum class Pull : uint8_t {
    None,
    Up,
    Down,
    UpDown
};

enum class InterruptTrigger : uint8_t {
    RisingEdge,
    FallingEdge,
    AnyEdge,
    LowLevel,
    HighLevel
};

struct PinConfiguration {
    Direction DirectionMode = Direction::Input;
    Pull PullMode = Pull::None;
    State InitialState = State::Low;
};

struct InterruptConfiguration {
    InterruptTrigger Trigger = InterruptTrigger::AnyEdge;
    ProcessorAffinity Affinity = ProcessorAffinity::Any();
    bool StartEnabled = true;
};

using InterruptCallback = void (*)(void* context);

class IInterrupt {
public:
    virtual ~IInterrupt() = default;

    virtual Pin GetPin() const noexcept = 0;
    virtual ProcessorAffinity GetAffinity() const noexcept = 0;
    virtual bool IsEnabled() const noexcept = 0;

    virtual PlatformResult Enable() noexcept = 0;
    virtual PlatformResult Disable() noexcept = 0;
};

using InterruptHandle = std::unique_ptr<IInterrupt>;

class IController {
public:
    virtual ~IController() = default;

    virtual PlatformResult Configure(
        Pin pin,
        const PinConfiguration& configuration
    ) noexcept = 0;

    virtual PlatformResult Write(Pin pin, State state) noexcept = 0;
    virtual PlatformResult Read(Pin pin, State& state) const noexcept = 0;

    virtual InterruptHandle CreateInterrupt(
        Pin pin,
        const InterruptConfiguration& configuration,
        InterruptCallback callback,
        void* context = nullptr
    ) = 0;

    virtual bool SupportsInterrupts() const noexcept = 0;
    virtual bool SupportsInterruptAffinity() const noexcept = 0;
};

inline IController*& ControllerStorage() noexcept {
    static IController* controller = nullptr;
    return controller;
}

inline IController* Controller() noexcept {
    return ControllerStorage();
}

inline void SetController(IController* controller) noexcept {
    ControllerStorage() = controller;
}

inline void ResetController() noexcept {
    ControllerStorage() = nullptr;
}

}
}
}
