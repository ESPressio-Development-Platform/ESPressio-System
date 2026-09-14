#include <cassert>
#include <cstdint>

#include <ESPressio_SystemClassificationIdentifiers.hpp>

using namespace ESPressio::System;

int main() {
    static_assert(sizeof(ProductTypeIdentifier) == 8U);
    static_assert(sizeof(HardwareFamilyIdentifier) == 8U);
    static_assert(sizeof(HardwareRevision) == 4U);
    static_assert(sizeof(ArchitectureIdentifier) == 8U);
    static_assert(sizeof(SoftwareVariantIdentifier) == 8U);

    constexpr ProductTypeIdentifier productA{1U};
    constexpr ProductTypeIdentifier productB{2U};
    constexpr HardwareFamilyIdentifier hardwareFamily{7U};
    constexpr HardwareRevision revision1{1U};
    constexpr HardwareRevision revision2{2U};
    constexpr ArchitectureIdentifier architecture{11U};
    constexpr SoftwareVariantIdentifier variant{19U};

    static_assert(bool(productA));
    static_assert(productA != productB);
    static_assert(productA < productB);
    static_assert(bool(hardwareFamily));
    static_assert(revision1 < revision2);
    static_assert(revision2 >= revision1);
    static_assert(bool(architecture));
    static_assert(bool(variant));

    assert(!ProductTypeIdentifier{});
    assert(!HardwareFamilyIdentifier{});
    assert(!HardwareRevision{});
    assert(!ArchitectureIdentifier{});
    assert(!SoftwareVariantIdentifier{});

    return 0;
}
