# Runtime identity validation

Starting branch: primitives_redesign at 6f59fd5d97726fd820ad422d94857ca941072145.

P4 System ownership: exact 4-byte nonzero incarnation and 20-byte device/runtime value; one process installation; acquire/release publication; immutable subsequent reads; no reset/provider lookup or Persistence dependency. The installing caller must supply a durably committed candidate. Persistence provisioning/allocation is a separate Foundation work package.

Existing test semantic review before execution:

| Test | Classification |
| --- | --- |
| device_identifier_test | Valid unchanged: stable device scalar, not runtime allocation |
| memory_policy_test | Valid unchanged: explicit general memory-provider and allocator ownership |
| platform_abstractions_test | Valid unchanged: execution, signal and monotonic abstractions |
| provider_publication_test | Valid unchanged: provider publication, independent of immutable identity |

All four existing tests and two new runtime identity tests compiled with GCC 13.3, C++17, warnings-as-errors and ran successfully. New tests cover unavailable output preservation, invalid inputs, duplicate and replacement refusal, maximum scalar value, single winner among eight installers, and concurrent readers never observing a partial identity. Changed headers also compile independently with RTTI/exceptions disabled. No claim of hardware power-loss validation is made here; durable allocation tests belong to Persistence.

Storage: exactly 20 semantic bytes plus one atomic publication state and target alignment. No heap allocation, signal, worker, storage provider or callback is used by installation/reads. No manifest dependency or version field changed.
