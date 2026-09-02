# Execution Provider Contract

An execution provider supplies the primitive execution semantics required by ESPressio without leaking native RTOS task handles.

The 1.0.0 contract exposes creation/destruction, suspend/resume, current execution identity, minimum free stack telemetry, processor count, sleep, yield, and processor-affinity capability reporting.

## Creation

`Create()` receives an `ExecutionEntry`, context pointer and `ExecutionConfiguration`, and returns an `ExecutionCreationResult` containing both an ESPressio `PlatformResult` and opaque `ExecutionHandle`.

A successful result must not return `InvalidExecutionHandle`.

## Configuration

The portable configuration expresses:

- name;
- stack size in bytes;
- priority;
- requested `ProcessorAffinity`.

Translate these semantic values into the target scheduler without leaking the target's native task/thread type.

## Capability reporting

`ProcessorCount()` and `SupportsProcessorAffinity()` are deliberately independent. Report the real target behaviour: do not infer enforceable affinity merely from the existence of multiple processors.

## Fallback semantics

When no implementation is installed, System uses an unavailable/null provider. A platform package installs its implementation through `SetProvider()` during startup.

## Testing

Test creation/destruction, lifecycle transitions, current identity, sleep/yield, stack telemetry, unrestricted execution, valid affinity requests, and unsupported/conflicting affinity behaviour.