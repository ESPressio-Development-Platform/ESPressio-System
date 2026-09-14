#pragma once

#include <cstdint>

namespace ESPressio::System {

/// <summary>Stable semantic identity of one product/device class. Zero is invalid/unspecified.</summary>
class ProductTypeIdentifier final {
    std::uint64_t value_{};
public:
    constexpr ProductTypeIdentifier() noexcept = default;
    constexpr explicit ProductTypeIdentifier(std::uint64_t value) noexcept : value_(value) {}
    constexpr std::uint64_t Value() const noexcept { return value_; }
    constexpr explicit operator bool() const noexcept { return value_ != 0U; }
    constexpr bool operator==(ProductTypeIdentifier other) const noexcept { return value_ == other.value_; }
    constexpr bool operator!=(ProductTypeIdentifier other) const noexcept { return !(*this == other); }
    constexpr bool operator<(ProductTypeIdentifier other) const noexcept { return value_ < other.value_; }
};
static_assert(sizeof(ProductTypeIdentifier) == 8U, "ProductTypeIdentifier must be exactly eight bytes");

/// <summary>Stable semantic identity of one hardware/board family. Zero is invalid/unspecified.</summary>
class HardwareFamilyIdentifier final {
    std::uint64_t value_{};
public:
    constexpr HardwareFamilyIdentifier() noexcept = default;
    constexpr explicit HardwareFamilyIdentifier(std::uint64_t value) noexcept : value_(value) {}
    constexpr std::uint64_t Value() const noexcept { return value_; }
    constexpr explicit operator bool() const noexcept { return value_ != 0U; }
    constexpr bool operator==(HardwareFamilyIdentifier other) const noexcept { return value_ == other.value_; }
    constexpr bool operator!=(HardwareFamilyIdentifier other) const noexcept { return !(*this == other); }
    constexpr bool operator<(HardwareFamilyIdentifier other) const noexcept { return value_ < other.value_; }
};
static_assert(sizeof(HardwareFamilyIdentifier) == 8U, "HardwareFamilyIdentifier must be exactly eight bytes");

/// <summary>Bounded ordered hardware revision within one HardwareFamilyIdentifier. Zero is invalid/unspecified.</summary>
class HardwareRevision final {
    std::uint32_t value_{};
public:
    constexpr HardwareRevision() noexcept = default;
    constexpr explicit HardwareRevision(std::uint32_t value) noexcept : value_(value) {}
    constexpr std::uint32_t Value() const noexcept { return value_; }
    constexpr explicit operator bool() const noexcept { return value_ != 0U; }
    constexpr bool operator==(HardwareRevision other) const noexcept { return value_ == other.value_; }
    constexpr bool operator!=(HardwareRevision other) const noexcept { return !(*this == other); }
    constexpr bool operator<(HardwareRevision other) const noexcept { return value_ < other.value_; }
    constexpr bool operator<=(HardwareRevision other) const noexcept { return value_ <= other.value_; }
    constexpr bool operator>(HardwareRevision other) const noexcept { return value_ > other.value_; }
    constexpr bool operator>=(HardwareRevision other) const noexcept { return value_ >= other.value_; }
};
static_assert(sizeof(HardwareRevision) == 4U, "HardwareRevision must be exactly four bytes");

/// <summary>Stable semantic identity of an execution/ABI architecture family. Zero is invalid/unspecified.</summary>
class ArchitectureIdentifier final {
    std::uint64_t value_{};
public:
    constexpr ArchitectureIdentifier() noexcept = default;
    constexpr explicit ArchitectureIdentifier(std::uint64_t value) noexcept : value_(value) {}
    constexpr std::uint64_t Value() const noexcept { return value_; }
    constexpr explicit operator bool() const noexcept { return value_ != 0U; }
    constexpr bool operator==(ArchitectureIdentifier other) const noexcept { return value_ == other.value_; }
    constexpr bool operator!=(ArchitectureIdentifier other) const noexcept { return !(*this == other); }
    constexpr bool operator<(ArchitectureIdentifier other) const noexcept { return value_ < other.value_; }
};
static_assert(sizeof(ArchitectureIdentifier) == 8U, "ArchitectureIdentifier must be exactly eight bytes");

/// <summary>Stable semantic identity of the selected software/product variant. Zero is invalid/unspecified.</summary>
class SoftwareVariantIdentifier final {
    std::uint64_t value_{};
public:
    constexpr SoftwareVariantIdentifier() noexcept = default;
    constexpr explicit SoftwareVariantIdentifier(std::uint64_t value) noexcept : value_(value) {}
    constexpr std::uint64_t Value() const noexcept { return value_; }
    constexpr explicit operator bool() const noexcept { return value_ != 0U; }
    constexpr bool operator==(SoftwareVariantIdentifier other) const noexcept { return value_ == other.value_; }
    constexpr bool operator!=(SoftwareVariantIdentifier other) const noexcept { return !(*this == other); }
    constexpr bool operator<(SoftwareVariantIdentifier other) const noexcept { return value_ < other.value_; }
};
static_assert(sizeof(SoftwareVariantIdentifier) == 8U, "SoftwareVariantIdentifier must be exactly eight bytes");

} // namespace ESPressio::System
