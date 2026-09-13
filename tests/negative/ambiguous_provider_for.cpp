#include "ESPressio_CompositionFramework.hpp"
namespace CF = ESPressio::System::CompositionFramework;
struct D : CF::Domain {};
struct S : CF::SharedCapability<D> {};
using Caps = CF::CapabilitySet<D,S>;
struct P1 : CF::ProviderDeclaration<D,Caps> {};
struct P2 : CF::ProviderDeclaration<D,Caps> {};
using C = CF::Composition<D,P1,P2>;
using Bad = C::ProviderFor<S>;
Bad* p = nullptr;
