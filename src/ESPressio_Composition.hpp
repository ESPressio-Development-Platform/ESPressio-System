#pragma once
#include "ESPressio_CompositionRequirements.hpp"

namespace ESPressio::System::CompositionFramework {
namespace Detail {
template<class D,class P,class=void> struct IsProvider:std::false_type{};
template<class D,class P> struct IsProvider<D,P,std::void_t<typename P::CompositionDomain,typename P::CompositionCapabilities,typename P::CompositionRequirements>>:std::bool_constant<std::is_same_v<typename P::CompositionDomain,D>>{};
template<class P,class C> inline constexpr bool ProviderProvidesV=P::CompositionCapabilities::template Contains<C>;
template<class C,class... Ps> inline constexpr std::size_t ProviderCountV=(std::size_t{0}+...+(ProviderProvidesV<Ps,C>?1U:0U));
template<class P,class E,class D,bool=ProviderProvidesV<P,typename RequirementEntry<D,E>::CapabilityType>> struct ProviderSatisfies:std::false_type{};
template<class P,class E,class D> struct ProviderSatisfies<P,E,D,true>:std::bool_constant<RequirementEntry<D,E>::template PropertiesSatisfied<typename P::CompositionCapabilities::template PropertiesFor<typename RequirementEntry<D,E>::CapabilityType>>>{};
template<class E,class D,class... Ps> inline constexpr std::size_t SatisfyingCountV=(std::size_t{0}+...+(ProviderSatisfies<Ps,E,D>::value?1U:0U));
template<class Set,class D,class... Ps> struct RequirementsSatisfied;
template<class D,class... Es,class... Ps> struct RequirementsSatisfied<RequirementSet<D,Es...>,D,Ps...>:std::bool_constant<((SatisfyingCountV<Es,D,Ps...> > 0U)&&...)>{};
template<class Set,class D,class... Ps> struct ConflictFree;
template<class D,class... Es,class... Ps> struct ConflictFree<CapabilitySet<D,Es...>,D,Ps...>:std::bool_constant<((!IsExclusiveCapabilityForV<D,typename CapabilityEntry<D,Es>::CapabilityType>||ProviderCountV<typename CapabilityEntry<D,Es>::CapabilityType,Ps...> <= 1U)&&...)>{};
template<class C,class... Ps> struct FirstProvider;
template<class C> struct FirstProvider<C>{using Type=void;};
template<class C,class F,class... R> struct FirstProvider<C,F,R...>{using Type=std::conditional_t<ProviderProvidesV<F,C>,F,typename FirstProvider<C,R...>::Type>;};
template<class C,class Acc,class... Ps> struct FilterProviders;
template<class C,class... A> struct FilterProviders<C,ProviderList<A...>>{using Type=ProviderList<A...>;};
template<class C,class... A,class F,class... R> struct FilterProviders<C,ProviderList<A...>,F,R...>{using Next=std::conditional_t<ProviderProvidesV<F,C>,ProviderList<A...,F>,ProviderList<A...>>;using Type=typename FilterProviders<C,Next,R...>::Type;};
template<class D,class T> struct IsRequirementSet:std::false_type{};
template<class D,class... Es> struct IsRequirementSet<D,RequirementSet<D,Es...>>:std::true_type{};
}

template<class D,class P> inline constexpr bool IsProviderForV=Detail::IsProvider<D,P>::value;

template<class D,class... Ps> struct Composition {
  static_assert(IsDomainV<D>); static_assert((IsProviderForV<D,Ps>&&...),"Composition contains invalid/cross-domain provider");
private:
  static constexpr bool NoConflicts=(Detail::ConflictFree<typename Ps::CompositionCapabilities,D,Ps...>::value&&...);
  static constexpr bool RequirementsOk=(Detail::RequirementsSatisfied<typename Ps::CompositionRequirements,D,Ps...>::value&&...);
public:
  using CompositionDomain=D;
  static_assert(NoConflicts,"Multiple providers for exclusive capability"); static_assert(RequirementsOk,"Unsatisfied provider requirement");
  static constexpr std::size_t ProviderCount=sizeof...(Ps); static constexpr bool IsValid=NoConflicts&&RequirementsOk;
  template<class C> static constexpr std::size_t ProviderCountFor=Detail::ProviderCountV<C,Ps...>;
  template<class C> static constexpr bool Provides=ProviderCountFor<C> > 0U;
  template<class C> using ProviderListFor=typename Detail::FilterProviders<C,ProviderList<>,Ps...>::Type;
  template<class E> static constexpr std::size_t ProvidersSatisfying=Detail::SatisfyingCountV<E,D,Ps...>;
  template<class R> static constexpr bool Satisfies=Detail::IsRequirementSet<D,R>::value && Detail::RequirementsSatisfied<R,D,Ps...>::value;
  template<class C> struct ResolveProvider { static_assert(ProviderCountFor<C> == 1U,"ProviderFor requires exactly one provider"); using Type=typename Detail::FirstProvider<C,Ps...>::Type; };
  template<class C> using ProviderFor=typename ResolveProvider<C>::Type;
  template<class P,class C> using PropertiesForProvider=typename P::CompositionCapabilities::template PropertiesFor<C>;
  template<class C> using PropertiesFor=PropertiesForProvider<ProviderFor<C>,C>;
  template<class P,class C,class K> static constexpr bool ProviderHasProperty=PropertiesForProvider<P,C>::template Contains<K>;
  template<class P,class C,class K> static constexpr typename K::ValueType ProviderPropertyValue=PropertiesForProvider<P,C>::template Value<K>;
  template<class C,class K> static constexpr bool HasProperty=PropertiesFor<C>::template Contains<K>;
  template<class C,class K> static constexpr typename K::ValueType PropertyValue=PropertiesFor<C>::template Value<K>;
};

template<class C,class R> struct Require { static_assert(C::template Satisfies<R>,"Composition does not satisfy requirements"); using Type=C; };
template<class C,class R> using RequireT=typename Require<C,R>::Type;

} // namespace ESPressio::System::CompositionFramework
