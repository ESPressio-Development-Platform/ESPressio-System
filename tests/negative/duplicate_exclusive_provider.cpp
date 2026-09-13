#include "ESPressio_CompositionFramework.hpp"
namespace CF = ESPressio::System::CompositionFramework;
struct D : CF::Domain {};
struct C : CF::ExclusiveCapability<D> {};
using Caps = CF::CapabilitySet<D,C>;
struct P1 : CF::ProviderDeclaration<D,Caps> {};
struct P2 : CF::ProviderDeclaration<D,Caps> {};
using Bad = CF::Composition<D,P1,P2>;
static_assert(Bad::IsValid);
