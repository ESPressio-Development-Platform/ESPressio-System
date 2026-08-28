# Implementing Providers

Provider implementations translate ESPressio semantics into target-native facilities.

## General requirements

A provider should:

- implement the ESPressio contract rather than expose the native API;
- map native failures into ESPressio result/status vocabulary;
- keep native handles opaque;
- report optional capabilities accurately;
- define ownership and destruction deterministically;
- remain valid for the lifetime required by objects that capture/reference it;
- avoid imposing a target dependency on System itself.

## Installation

Capabilities with replaceable provider slots are installed during platform startup, before dependent long-lived objects are created.

## Do not broaden System unnecessarily

When adding hardware support, first ask whether the abstraction is genuinely platform/runtime-level or belongs to a feature domain. A radio protocol, WiFi lifecycle, event router, or command dispatcher is not automatically a System concern merely because it eventually uses hardware.

## Provider-specific contracts

- [Memory Provider Contract](Memory-Provider-Contract)
- [Execution Provider Contract](Execution-Provider-Contract)
- [GPIO Provider Contract](GPIO-Provider-Contract)
- [Clock Provider Contract](Clock-Provider-Contract)