#pragma once
#include "ESPressio_CompositionTypes.hpp"

namespace ESPressio::System::CompositionFramework {

template<class D> struct Constraint { static_assert(IsDomainV<D>); using CompositionDomain=D; };
template<class D,class T> inline constexpr bool IsConstraintForV=std::is_base_of_v<Constraint<D>,T> && !std::is_same_v<Constraint<D>,T>;

namespace Detail {
template<class S,class P,auto E> constexpr bool Eq(){if constexpr(!S::template Contains<P>) return false; else return S::template Value<P> == static_cast<typename P::ValueType>(E);}
template<class S,class P,auto E> constexpr bool Ge(){if constexpr(!S::template Contains<P>) return false; else return S::template Value<P> >= static_cast<typename P::ValueType>(E);}
template<class S,class P,auto E> constexpr bool Le(){if constexpr(!S::template Contains<P>) return false; else return S::template Value<P> <= static_cast<typename P::ValueType>(E);}
template<class S,class P,auto E> constexpr bool Gt(){if constexpr(!S::template Contains<P>) return false; else return S::template Value<P> > static_cast<typename P::ValueType>(E);}
template<class S,class P,auto E> constexpr bool Lt(){if constexpr(!S::template Contains<P>) return false; else return S::template Value<P> < static_cast<typename P::ValueType>(E);}
}

template<class D,class P,auto E> struct PropertyEquals:Constraint<D>{static_assert(IsPropertyForV<D,P>);template<class S>static constexpr bool Satisfied=Detail::Eq<S,P,E>();};
template<class D,class P,auto E> struct PropertyAtLeast:Constraint<D>{static_assert(IsPropertyForV<D,P>);template<class S>static constexpr bool Satisfied=Detail::Ge<S,P,E>();};
template<class D,class P,auto E> struct PropertyAtMost:Constraint<D>{static_assert(IsPropertyForV<D,P>);template<class S>static constexpr bool Satisfied=Detail::Le<S,P,E>();};
template<class D,class P,auto E> struct PropertyGreaterThan:Constraint<D>{static_assert(IsPropertyForV<D,P>);template<class S>static constexpr bool Satisfied=Detail::Gt<S,P,E>();};
template<class D,class P,auto E> struct PropertyLessThan:Constraint<D>{static_assert(IsPropertyForV<D,P>);template<class S>static constexpr bool Satisfied=Detail::Lt<S,P,E>();};

template<class D,class C,class... Cs> struct CapabilityRequirement {
  static_assert(IsCapabilityForV<D,C>,"Requirement capability belongs to another domain"); static_assert((IsConstraintForV<D,Cs>&&...),"Requirement constraint belongs to another domain");
  using CompositionDomain=D; using RequirementTag=void; using CapabilityType=C;
  template<class S> static constexpr bool PropertiesSatisfied=(Cs::template Satisfied<S>&&...);
};

namespace Detail {
template<class D,class E,class=void> struct RequirementEntry { static constexpr bool Valid=IsCapabilityForV<D,E>; using CapabilityType=E; template<class> static constexpr bool PropertiesSatisfied=true; };
template<class D,class E> struct RequirementEntry<D,E,std::void_t<typename E::RequirementTag,typename E::CapabilityType>> { static constexpr bool Valid=std::is_same_v<typename E::CompositionDomain,D> && IsCapabilityForV<D,typename E::CapabilityType>; using CapabilityType=typename E::CapabilityType; template<class S> static constexpr bool PropertiesSatisfied=E::template PropertiesSatisfied<S>; };
template<class D,class... Es> struct UniqueRequirements;
template<class D> struct UniqueRequirements<D>:std::true_type{};
template<class D,class F,class... R> struct UniqueRequirements<D,F,R...>:std::bool_constant<((!std::is_same_v<typename RequirementEntry<D,F>::CapabilityType,typename RequirementEntry<D,R>::CapabilityType>)&&...)&&UniqueRequirements<D,R...>::value>{};
}

template<class D,class... Es> struct RequirementSet {
  static_assert(IsDomainV<D>); static_assert((Detail::RequirementEntry<D,Es>::Valid&&...),"RequirementSet contains invalid/cross-domain entry");
  static_assert(Detail::UniqueRequirements<D,Es...>::value,"Duplicate requirement key"); using CompositionDomain=D; static constexpr std::size_t Count=sizeof...(Es);
};

template<class D,class Caps,class Reqs=RequirementSet<D>> struct ProviderDeclaration {
  static_assert(IsDomainV<D>); static_assert(std::is_same_v<typename Caps::CompositionDomain,D>,"Provider capabilities belong to another domain");
  static_assert(std::is_same_v<typename Reqs::CompositionDomain,D>,"Provider requirements belong to another domain");
  using CompositionDomain=D; using CompositionCapabilities=Caps; using CompositionRequirements=Reqs;
};

} // namespace ESPressio::System::CompositionFramework
