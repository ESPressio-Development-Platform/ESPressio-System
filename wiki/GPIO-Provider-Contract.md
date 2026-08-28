# GPIO Provider Contract

A GPIO provider maps ESPressio pin configuration and interrupt semantics to the target's GPIO facilities.

## Required concerns

Implementations must preserve pin configuration, direction, pull configuration, digital state, interrupt trigger semantics, interrupt ownership/lifecycle, and optional processor affinity.

## Interrupt lifecycle

Creation returns an explicit result and a move-only owning handle. Destruction of that handle must release the native registration. Enable/disable should change active state without transferring ownership.

## Affinity

Specific interrupt affinity is not assumed. Report support accurately and return explicit unsupported/conflict status when the requested semantics cannot be satisfied.

## ISR safety

Native interrupt callbacks must obey the target SDK's ISR restrictions. The provider is responsible for translating ESPressio's interrupt-context operations onto facilities that are actually safe on that target.