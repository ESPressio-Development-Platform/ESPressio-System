#include "ESPressio_CompositionFramework.hpp"
namespace CF = ESPressio::System::CompositionFramework;
struct D : CF::Domain {};
struct A : CF::ExclusiveCapability<D> {};
struct B : CF::ExclusiveCapability<D> {};
using Caps = CF::CapabilitySet<D,A>;
using Reqs = CF::RequirementSet<D,B>;
struct P : CF::ProviderDeclaration<D,Caps,Reqs> {};
using Bad = CF::Composition<D,P>;
static_assert(Bad::IsValid);
