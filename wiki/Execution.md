# Execution

ESPressio System defines the primitive execution capability used beneath higher-level scheduling and threading libraries.

The abstraction covers execution creation/destruction, suspend/resume, current execution identity, stack telemetry, processor-count discovery, sleep, yield, and optional processor affinity.

Native RTOS task handles do not form part of the public contract.

## Processor affinity

`ProcessorAffinity` can express either any processor or a requested processor/core. Multi-processor support and affinity support are separate capabilities: a target can expose multiple processors without guaranteeing explicit placement.

Consumers should therefore treat requested affinity as a capability-dependent request rather than assuming it is universally enforceable.

## Higher-level execution

Application developers will commonly use ESPressio Task or ESPressio Threads rather than this primitive interface directly. System exists so those libraries can remain independent of FreeRTOS or another native scheduler.

Platform implementers should continue with [Execution Provider Contract](Execution-Provider-Contract).