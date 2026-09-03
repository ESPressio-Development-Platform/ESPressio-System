#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace ESPressio {
namespace System {

/// <summary>
/// Permanent transport-independent 128-bit identity for one ESPressio device.
/// </summary>
/// <remarks>
/// DeviceIdentifier is identity, not an address or authentication credential.
/// Concrete platform libraries are responsible for deriving a stable default
/// identifier from platform identity where appropriate. Applications may also
/// provide an explicitly persisted identifier.
/// </remarks>
class DeviceIdentifier final {
public:
    /// <summary>Identifier width in bytes.</summary>
    static constexpr std::size_t Size = 16;

    /// <summary>Underlying fixed-width byte representation.</summary>
    using Storage = std::array<std::uint8_t, Size>;

private:
    Storage _bytes{};

public:
    /// <summary>Creates the Invalid/Unspecified all-zero identifier.</summary>
    constexpr DeviceIdentifier() noexcept = default;

    /// <summary>Creates an identifier from its complete 128-bit representation.</summary>
    /// <param name="bytes">Exact identifier bytes.</param>
    constexpr explicit DeviceIdentifier(const Storage& bytes) noexcept : _bytes(bytes) {}

    /// <summary>Gets the complete immutable identifier representation.</summary>
    constexpr const Storage& Bytes() const noexcept { return _bytes; }

    /// <summary>Indicates whether this value is Invalid/Unspecified.</summary>
    constexpr bool IsZero() const noexcept {
        for (const auto value : _bytes) {
            if (value != 0U) return false;
        }
        return true;
    }

    /// <summary>Indicates whether this value is a valid non-zero device identity.</summary>
    constexpr explicit operator bool() const noexcept { return !IsZero(); }

    /// <summary>Compares identifiers by exact byte equality.</summary>
    constexpr bool operator==(const DeviceIdentifier& other) const noexcept {
        for (std::size_t index = 0; index < Size; ++index) {
            if (_bytes[index] != other._bytes[index]) return false;
        }
        return true;
    }

    /// <summary>Compares identifiers for inequality.</summary>
    constexpr bool operator!=(const DeviceIdentifier& other) const noexcept {
        return !(*this == other);
    }

    /// <summary>
    /// Provides deterministic lexicographic ordering suitable for bounded
    /// ordered collections and distributed deterministic tie-breaking.
    /// </summary>
    constexpr bool operator<(const DeviceIdentifier& other) const noexcept {
        for (std::size_t index = 0; index < Size; ++index) {
            if (_bytes[index] < other._bytes[index]) return true;
            if (_bytes[index] > other._bytes[index]) return false;
        }
        return false;
    }
};

static_assert(sizeof(DeviceIdentifier) == DeviceIdentifier::Size,
              "DeviceIdentifier must remain an exact 16-byte semantic value.");

} // namespace System
} // namespace ESPressio
