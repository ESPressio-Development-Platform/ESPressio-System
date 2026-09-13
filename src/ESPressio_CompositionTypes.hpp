#pragma once
#include <cstddef>
#include <type_traits>

namespace ESPressio::System::CompositionFramework {

struct Domain {};
template<class D,class... Es> struct PropertySet;
template <class T> inline constexpr bool IsDomainV = std::is_base_of_v<Domain,T> && !std::is_same_v<Domain,T>;

template <class D> struct ExclusiveCapability { static_assert(IsDomainV<D>); using CompositionDomain=D; };
template <class D> struct SharedCapability { static_assert(IsDomainV<D>); using CompositionDomain=D; };
template <class D,class V> struct Property { static_assert(IsDomainV<D>); using CompositionDomain=D; using ValueType=V; };

namespace Detail {
template<class T,class=void> struct CapabilityTraits { using DomainType=void; static constexpr bool Valid=false,Exclusive=false,Shared=false; };
template<class T> struct CapabilityTraits<T,std::void_t<typename T::CompositionDomain>> {
  using DomainType=typename T::CompositionDomain;
  static constexpr bool Exclusive=IsDomainV<DomainType> && std::is_base_of_v<ExclusiveCapability<DomainType>,T>;
  static constexpr bool Shared=IsDomainV<DomainType> && std::is_base_of_v<SharedCapability<DomainType>,T>;
  static constexpr bool Valid=Exclusive||Shared;
};
template<class T,class=void> struct PropertyTraits { using DomainType=void; using ValueType=void; static constexpr bool Valid=false; };
template<class T> struct PropertyTraits<T,std::void_t<typename T::CompositionDomain,typename T::ValueType>> {
  using DomainType=typename T::CompositionDomain; using ValueType=typename T::ValueType;
  static constexpr bool Valid=IsDomainV<DomainType> && std::is_base_of_v<Property<DomainType,ValueType>,T>;
};
template<class P,class... Es> struct FindProperty;
template<class P> struct FindProperty<P>{using Type=void;};
template<class P,class F,class... R> struct FindProperty<P,F,R...>{using Type=std::conditional_t<std::is_same_v<P,typename F::PropertyType>,F,typename FindProperty<P,R...>::Type>;};
template<class... Es> struct UniqueProperties;
template<> struct UniqueProperties<>:std::true_type{};
template<class F,class... R> struct UniqueProperties<F,R...>:std::bool_constant<((!std::is_same_v<typename F::PropertyType,typename R::PropertyType>)&&...)&&UniqueProperties<R...>::value>{};
template<class D,class E,class=void> struct CapabilityEntry { static constexpr bool Valid=false; using CapabilityType=void; using Properties=PropertySet<D>; };
template<class D,class E> struct CapabilityEntry<D,E,std::enable_if_t<CapabilityTraits<E>::Valid>> { static constexpr bool Valid=std::is_same_v<typename CapabilityTraits<E>::DomainType,D>; using CapabilityType=E; using Properties=PropertySet<D>; };
template<class D,class E> struct CapabilityEntry<D,E,std::void_t<typename E::CapabilityType,typename E::Properties>> { static constexpr bool Valid=std::is_same_v<typename E::CompositionDomain,D> && CapabilityTraits<typename E::CapabilityType>::Valid && std::is_same_v<typename CapabilityTraits<typename E::CapabilityType>::DomainType,D>; using CapabilityType=typename E::CapabilityType; using Properties=typename E::Properties; };
template<class D,class... Es> struct UniqueCapabilities;
template<class D> struct UniqueCapabilities<D>:std::true_type{};
template<class D,class F,class... R> struct UniqueCapabilities<D,F,R...>:std::bool_constant<((!std::is_same_v<typename CapabilityEntry<D,F>::CapabilityType,typename CapabilityEntry<D,R>::CapabilityType>)&&...)&&UniqueCapabilities<D,R...>::value>{};
template<class D,class C,class... Es> struct FindCapability;
template<class D,class C> struct FindCapability<D,C>{using Type=void;};
template<class D,class C,class F,class... R> struct FindCapability<D,C,F,R...>{using Type=std::conditional_t<std::is_same_v<C,typename CapabilityEntry<D,F>::CapabilityType>,F,typename FindCapability<D,C,R...>::Type>;};
template<class T,class=void> struct IsPropertyValue:std::false_type{};
template<class T> struct IsPropertyValue<T,std::void_t<typename T::CompositionDomain,typename T::PropertyType,typename T::ValueType>>:std::true_type{};
template<class N,class... H> inline constexpr bool ContainsTypeV=(std::is_same_v<N,H>||...);
}

template<class D,class T> inline constexpr bool IsCapabilityForV=Detail::CapabilityTraits<T>::Valid && std::is_same_v<typename Detail::CapabilityTraits<T>::DomainType,D>;
template<class D,class T> inline constexpr bool IsExclusiveCapabilityForV=IsCapabilityForV<D,T> && Detail::CapabilityTraits<T>::Exclusive;
template<class D,class T> inline constexpr bool IsSharedCapabilityForV=IsCapabilityForV<D,T> && Detail::CapabilityTraits<T>::Shared;
template<class D,class T> inline constexpr bool IsPropertyForV=Detail::PropertyTraits<T>::Valid && std::is_same_v<typename Detail::PropertyTraits<T>::DomainType,D>;

template<class D,class P,auto V> struct PropertyValue {
  static_assert(IsPropertyForV<D,P>,"Property belongs to another composition domain");
  static_assert(std::is_convertible_v<decltype(V),typename P::ValueType>,"Property value type mismatch");
  using CompositionDomain=D; using PropertyType=P; using ValueType=typename P::ValueType;
  static constexpr ValueType Value=static_cast<ValueType>(V);
};

template<class D,class... Es> struct PropertySet {
  static_assert(IsDomainV<D>); static_assert((Detail::IsPropertyValue<Es>::value&&...),"PropertySet entries must be PropertyValue types");
  static_assert((std::is_same_v<typename Es::CompositionDomain,D>&&...),"PropertySet contains another domain");
  static_assert(Detail::UniqueProperties<Es...>::value,"Duplicate property key");
  using CompositionDomain=D; static constexpr std::size_t Count=sizeof...(Es);
  template<class P> static constexpr bool Contains=!std::is_void_v<typename Detail::FindProperty<P,Es...>::Type>;
  template<class P> struct Get { static_assert(IsPropertyForV<D,P>); using E=typename Detail::FindProperty<P,Es...>::Type; static_assert(!std::is_void_v<E>,"Property not provided"); static constexpr typename P::ValueType Value=E::Value; };
  template<class P> static constexpr typename P::ValueType Value=Get<P>::Value;
};

template<class D,class C,class... Ps> struct CapabilityProfile {
  static_assert(IsCapabilityForV<D,C>,"Capability belongs to another composition domain");
  using CompositionDomain=D; using CapabilityType=C; using Properties=PropertySet<D,Ps...>;
};

template<class D,class... Es> struct CapabilitySet {
  static_assert(IsDomainV<D>); static_assert((Detail::CapabilityEntry<D,Es>::Valid&&...),"CapabilitySet contains invalid/cross-domain entry");
  static_assert(Detail::UniqueCapabilities<D,Es...>::value,"Duplicate capability key");
  using CompositionDomain=D; static constexpr std::size_t Count=sizeof...(Es);
  template<class C> static constexpr bool Contains=!std::is_void_v<typename Detail::FindCapability<D,C,Es...>::Type>;
  template<class C> struct Profile { static_assert(IsCapabilityForV<D,C>); using E=typename Detail::FindCapability<D,C,Es...>::Type; static_assert(!std::is_void_v<E>,"Capability not provided"); using Properties=typename Detail::CapabilityEntry<D,E>::Properties; };
  template<class C> using PropertiesFor=typename Profile<C>::Properties;
};

template<class... Ps> struct ProviderList { static constexpr std::size_t Count=sizeof...(Ps); template<class P> static constexpr bool Contains=Detail::ContainsTypeV<P,Ps...>; };

} // namespace ESPressio::System::CompositionFramework
