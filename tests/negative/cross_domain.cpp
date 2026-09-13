#include "ESPressio_CompositionFramework.hpp"
namespace CF = ESPressio::System::CompositionFramework;
struct A : CF::Domain {};
struct B : CF::Domain {};
struct AC : CF::ExclusiveCapability<A> {};
using Bad = CF::CapabilitySet<B, AC>;
Bad bad;
