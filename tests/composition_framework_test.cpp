#include "ESPressio_CompositionFramework.hpp"
#include <cstdint>
#include <type_traits>

namespace CF = ESPressio::System::CompositionFramework;

struct AlphaDomain final : CF::Domain {};
struct BetaDomain final : CF::Domain {};
struct AExclusive final : CF::ExclusiveCapability<AlphaDomain> {};
struct AShared final : CF::SharedCapability<AlphaDomain> {};
struct BExclusive final : CF::ExclusiveCapability<BetaDomain> {};
struct Capacity final : CF::Property<AlphaDomain, std::size_t> {};

using ACaps1 = CF::CapabilitySet<AlphaDomain,
    CF::CapabilityProfile<AlphaDomain, AExclusive, CF::PropertyValue<AlphaDomain, Capacity, 8>>>;
using ACaps2 = CF::CapabilitySet<AlphaDomain, AShared>;
using AReqShared = CF::RequirementSet<AlphaDomain, AShared>;
using AReqCapacity = CF::RequirementSet<AlphaDomain,
    CF::CapabilityRequirement<AlphaDomain, AExclusive, CF::PropertyAtLeast<AlphaDomain, Capacity, 4>>>;

struct P1 : CF::ProviderDeclaration<AlphaDomain, ACaps1> {};
struct P2 : CF::ProviderDeclaration<AlphaDomain, ACaps2> {};
struct P3 : CF::ProviderDeclaration<AlphaDomain, ACaps2, AReqCapacity> {};

using Empty = CF::Composition<AlphaDomain>;
using C = CF::Composition<AlphaDomain, P1, P2, P3>;

static_assert(Empty::IsValid && Empty::ProviderCount == 0);
static_assert(C::IsValid);
static_assert(C::ProviderCountFor<AExclusive> == 1);
static_assert(C::ProviderCountFor<AShared> == 2);
static_assert(std::is_same_v<C::ProviderFor<AExclusive>, P1>);
using Shared = C::ProviderListFor<AShared>;
static_assert(Shared::Count == 2 && Shared::Contains<P2> && Shared::Contains<P3>);
static_assert(C::Satisfies<AReqShared>);
static_assert(C::Satisfies<AReqCapacity>);
using Checked = CF::RequireT<C, AReqCapacity>;
static_assert(std::is_same_v<Checked, C>);
static_assert(C::PropertyValue<AExclusive, Capacity> == 8);
static_assert(C::ProviderPropertyValue<P1, AExclusive, Capacity> == 8);
static_assert(CF::IsCapabilityForV<AlphaDomain, AExclusive>);
static_assert(!CF::IsCapabilityForV<BetaDomain, AExclusive>);
static_assert(CF::IsCapabilityForV<BetaDomain, BExclusive>);

int main() { return 0; }
