# Getting Started

ESPressio System 1.0.0 is the platform-neutral capability layer beneath higher-level ESPressio libraries.

Most application code should consume System indirectly. Use it directly when you need portable access to memory policy, primitive execution, synchronization, bounded queues, clocks, GPIO, byte streams, entropy, or related runtime facilities.

## Include the complete surface

```cpp
#include <ESPressio_System.hpp>
```

Individual capability headers may be included when preferred.

## Provider model

System supplies abstractions and portable fallbacks where appropriate. A target integration installs concrete providers. This keeps native SDK and RTOS types below the abstraction boundary.

For ESP32 deployments, the concrete implementations belong in the ESPressio ESP32 platform library rather than System itself.

## Next steps

- [Memory Policies](Memory-Policies)
- [Execution](Execution)
- [Synchronization and Queues](Synchronization-and-Queues)
- [Clocks](Clocks)
- [GPIO](GPIO)
- [API Overview](API-Overview)

Developers implementing a new platform should instead begin with [Extension Architecture](Extension-Architecture).