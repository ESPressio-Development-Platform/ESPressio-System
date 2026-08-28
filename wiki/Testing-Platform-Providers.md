# Testing Platform Providers

Provider testing should demonstrate both contract correctness and target-specific correctness.

## Contract tests

Validate portable semantics independently of implementation details: lifecycle, ownership, status mapping, capability reporting, timeout/non-blocking behaviour, and unsupported operations.

## Target tests

Validate the mapping onto the actual SDK/RTOS/hardware, including memory placement, alignment, interrupt safety, affinity behaviour, timing monotonicity, counter lifecycle, queue behaviour, and resource cleanup as applicable.

## Failure paths matter

Tests should intentionally exercise unsupported capabilities, allocation failure, invalid/conflicting affinity, timeouts, lifecycle teardown, and provider replacement where the contract permits it.

A provider is complete only when its failure semantics are as deterministic as its successful path.