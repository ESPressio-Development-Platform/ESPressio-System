#include <array>
#include <cassert>
#include <cstdint>
#include <type_traits>

#include "ESPressio_DeviceIdentifier.hpp"

int main() {
    using ESPressio::System::DeviceIdentifier;

    static_assert(sizeof(DeviceIdentifier) == 16);
    static_assert(std::is_trivially_copyable<DeviceIdentifier>::value);

    DeviceIdentifier invalid;
    assert(invalid.IsZero());
    assert(!static_cast<bool>(invalid));

    DeviceIdentifier::Storage firstBytes{};
    firstBytes[0] = 0x01;
    firstBytes[15] = 0x7F;
    const DeviceIdentifier first(firstBytes);

    assert(!first.IsZero());
    assert(static_cast<bool>(first));
    assert(first.Bytes() == firstBytes);
    assert(first == DeviceIdentifier(firstBytes));
    assert(first != invalid);

    DeviceIdentifier::Storage secondBytes = firstBytes;
    secondBytes[15] = 0x80;
    const DeviceIdentifier second(secondBytes);
    assert(first < second);
    assert(!(second < first));

    return 0;
}
