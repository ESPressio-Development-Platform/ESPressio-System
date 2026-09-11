#pragma once
#include <cstdint>

namespace ESPressio::System {
/// <summary>Exact runtime lineage scalar. Zero is invalid; UINT32_MAX is the final valid incarnation.</summary>
/// <remarks>Persistence allocates durably once per actual process boot. This value does not
/// generate, wrap, reset, or allocate an identity when a service restarts.</remarks>
class RuntimeIncarnationId final {
    std::uint32_t value_{};
public:
    constexpr RuntimeIncarnationId() noexcept = default;
    /// <summary>Constructs a value from a validated durable allocator result.</summary>
    constexpr explicit RuntimeIncarnationId(std::uint32_t value) noexcept : value_(value) {}
    constexpr std::uint32_t Value() const noexcept { return value_; }
    constexpr explicit operator bool() const noexcept { return value_ != 0; }
    constexpr bool operator==(RuntimeIncarnationId other) const noexcept { return value_ == other.value_; }
    constexpr bool operator!=(RuntimeIncarnationId other) const noexcept { return !(*this == other); }
    constexpr bool operator<(RuntimeIncarnationId other) const noexcept { return value_ < other.value_; }
};
static_assert(sizeof(RuntimeIncarnationId) == 4, "RuntimeIncarnationId must be exactly four bytes");
} // namespace ESPressio::System
