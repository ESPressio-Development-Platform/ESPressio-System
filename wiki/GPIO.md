# GPIO

The GPIO abstraction provides portable vocabulary for pin identity, direction, pulls, digital state, interrupt triggers, interrupt lifecycle, and optional processor affinity.

```cpp
#include <ESPressio_GPIO.hpp>
using namespace ESPressio::System::GPIO;

auto* gpio = Controller();
if (gpio) {
    gpio->Configure(26, {Direction::Output, Pull::None, State::Low});
    gpio->Write(26, State::High);
}
```

## Interrupt ownership

Interrupt creation returns status together with a move-only RAII handle. The handle owns the registration; destroying or resetting it detaches and destroys the underlying interrupt through the provider.

`Enable()` and `Disable()` allow ownership to remain while the registration is temporarily inactive.

Specific processor affinity is a request. Providers report whether interrupt affinity is supported and can return an explicit unsupported/conflict result rather than collapsing failure into a null handle.

Platform implementers should continue with [GPIO Provider Contract](GPIO-Provider-Contract).