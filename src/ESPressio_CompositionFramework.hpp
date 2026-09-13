#pragma once

#include <cstddef>
#include <type_traits>

namespace ESPressio::System::CompositionFramework {

struct Domain {};

template <typename T>
inline constexpr bool IsDomainV = std::is_base_of_v<Domain, T> && !std::is_same_v<Domain, T>;

template <typename TDomain>
struct ExclusiveCapability {
    static_assert(IsDomainV<TDomain>, "Composition capability domain must derive from CompositionFramework::Domain");
    using CompositionDomain = TDomain;
};

template <typename TDomain>
struct SharedCapability {
    static_assert(IsDomainV<TDomain>, "Composition capability domain must derive from CompositionFramework::Domain");
    using CompositionDomain = TDomain;
};

template <typename TDomain, typename T>
struct Property {
    static_assert(IsDomainV<TDomain>, "Composition property domain must derive from CompositionFramework::Domain");
    using CompositionDomain = TDomain;
    using ValueType = T;
};

namespace Detail {

template <typename...>
struct TypesAreUnique : std::true_type {};

template <typename T, typename... TRest>
struct TypesAreUnique<T, TRest...>
    : std::bool_constant<(!std::is_same_v<T, TRest> && ...) && TypesAreUnique<TRest...>::value> {};

template <typename TNeedle, typename... THaystack>
inline constexpr bool ContainsTypeV = (std::is_same_v<TNeedle, THaystack> || ...);

template <typename T, typename = void>
struct CapabilityTraits {
    using DomainType = void;
    static constexpr bool IsExclusive = false;
    static constexpr bool IsShared = false;
    static constexpr bool IsValid = false;
};

template <typename T>
struct CapabilityTraits<T, std::void_t<typename T::CompositionDomain>> {
    using DomainType = typename T::CompositionDomain;
    static constexpr bool IsExclusive = std::is_base_of_v<ExclusiveCapability<DomainType>, T>;
    static constexpr bool IsShared = std::is_base_of_v<SharedCapability<DomainType>, T>;
    static constexpr bool IsValid = IsDomainV<DomainType> && (IsExclusive || IsShared);
};

template <typename T, typename = void>
struct PropertyTraits {
    using DomainType = void;
    using ValueType = void;
    static constexpr bool IsValid = false;
};

template <typename T>
struct PropertyTraits<T, std::void_t<typename T::CompositionDomain, typename T::ValueType>> {
    using DomainType = typename T::CompositionDomain;
    using ValueType = typename T::ValueType;
    static constexpr bool IsValid = IsDomainV<DomainType> && std::is_base_of_v<Property<DomainType, ValueType>, T>;
};

} // namespace Detail

template <typename TDomain, typename T>
inline constexpr bool IsCapabilityForV =
    Detail::CapabilityTraits<T>::IsValid && std::is_same_v<typename Detail::CapabilityTraits<T>::DomainType, TDomain>;

template <typename TDomain, typename T>
inline constexpr bool IsExclusiveCapabilityForV = IsCapabilityForV<TDomain, T> && Detail::CapabilityTraits<T>::IsExclusive;

template <typename TDomain, typename T>
inline constexpr bool IsSharedCapabilityForV = IsCapabilityForV<TDomain, T> && Detail::CapabilityTraits<T>::IsShared;

template <typename TDomain, typename T>
inline constexpr bool IsPropertyForV =
    Detail::PropertyTraits<T>::IsValid && std::is_same_v<typename Detail::PropertyTraits<T>::DomainType, TDomain>;

template <typename TDomain, typename TProperty, auto TValue>
struct PropertyValue {
    static_assert(IsPropertyForV<TDomain, TProperty>, "PropertyValue property must belong to this composition domain");
    static_assert(std::is_convertible_v<decltype(TValue), typename TProperty::ValueType>,
                  "PropertyValue value must be convertible to the property's ValueType");
    using CompositionDomain = TDomain;
    using PropertyType = TProperty;
    using ValueType = typename TProperty::ValueType;
    static constexpr ValueType Value = static_cast<ValueType>(TValue);
};

namespace Detail {

template <typename T, typename = void>
struct IsPropertyValue : std::false_type {};

template <typename T>
struct IsPropertyValue<T, std::void_t<typename T::CompositionDomain, typename T::PropertyType, typename T::ValueType>>
    : std::true_type {};

template <typename... TPropertyValues>
struct PropertyKeysAreUnique;

template <>
struct PropertyKeysAreUnique<> : std::true_type {};

template <typename TFirst, typename... TRest>
struct PropertyKeysAreUnique<TFirst, TRest...>
    : std::bool_constant<((!std::is_same_v<typename TFirst::PropertyType, typename TRest::PropertyType>) && ...) &&
                         PropertyKeysAreUnique<TRest...>::value> {};

template <typename TProperty, typename... TPropertyValues>
struct FindPropertyValue;

template <typename TProperty>
struct FindPropertyValue<TProperty> { using Type = void; };

template <typename TProperty, typename TFirst, typename... TRest>
struct FindPropertyValue<TProperty, TFirst, TRest...> {
    using Type = std::conditional_t<std::is_same_v<TProperty, typename TFirst::PropertyType>,
                                    TFirst,
                                    typename FindPropertyValue<TProperty, TRest...>::Type>;
};

} // namespace Detail

template <typename TDomain, typename... TPropertyValues>
struct PropertySet {
    static_assert(IsDomainV<TDomain>, "PropertySet domain must derive from CompositionFramework::Domain");
    static_assert((Detail::IsPropertyValue<TPropertyValues>::value && ...), "PropertySet entries must be PropertyValue types");
    static_assert((std::is_same_v<typename TPropertyValues::CompositionDomain, TDomain> && ...),
                  "PropertySet entries must belong to this composition domain");
    static_assert(Detail::PropertyKeysAreUnique<TPropertyValues...>::value,
                  "PropertySet must not contain more than one value for the same property key");

    using CompositionDomain = TDomain;
    static constexpr std::size_t Count = sizeof...(TPropertyValues);

    template <typename TProperty>
    static constexpr bool Contains =
        !std::is_void_v<typename Detail::FindPropertyValue<TProperty, TPropertyValues...>::Type>;

    template <typename TProperty>
    struct Get {
        static_assert(IsPropertyForV<TDomain, TProperty>, "Requested property must belong to this composition domain");
        using Entry = typename Detail::FindPropertyValue<TProperty, TPropertyValues...>::Type;
        static_assert(!std::is_void_v<Entry>, "Requested property is not declared by this provider");
        static constexpr typename TProperty::ValueType Value = Entry::Value;
    };

    template <typename TProperty>
    static constexpr typename TProperty::ValueType Value = Get<TProperty>::Value;
};

template <typename TDomain, typename TCapability, typename... TPropertyValues>
struct CapabilityProfile {
    static_assert(IsCapabilityForV<TDomain, TCapability>, "CapabilityProfile capability must belong to this composition domain");
    using CompositionDomain = TDomain;
    using CapabilityType = TCapability;
    using Properties = PropertySet<TDomain, TPropertyValues...>;
};

namespace Detail {

template <typename TDomain, typename T, typename = void>
struct CapabilityEntryTraits {
    static constexpr bool IsValid = IsCapabilityForV<TDomain, T>;
    using CapabilityType = T;
    using Properties = PropertySet<TDomain>;
};

template <typename TDomain, typename T>
struct CapabilityEntryTraits<TDomain, T, std::void_t<typename T::CapabilityType, typename T::Properties>> {
    static constexpr bool IsValid = IsCapabilityForV<TDomain, typename T::CapabilityType> &&
                                    std::is_same_v<typename T::CompositionDomain, TDomain>;
    using CapabilityType = typename T::CapabilityType;
    using Properties = typename T::Properties;
};

template <typename TDomain, typename... TEntries>
struct CapabilityKeysAreUnique;

template <typename TDomain>
struct CapabilityKeysAreUnique<Tdomain> : std::true_type {};

template <typename TDomain, typename TFirst, typename... TRest>
struct CapabilityKeysAreUnique<TDomain, TFirst, TRest...>
    : std::bool_constant<((!std::is_same_v<typename CapabilityEntryTraits<TDomain, TFirst>::CapabilityType,
                                             typename CapabilityEntryTraits<TDomain, TRest>::CapabilityType>) && ...) &&
                         CapabilityKeysAreUnique<TDomain, TRest...>::value> {};

template <typename TDomain, typename TCapability, typename... TEntries>
struct FindCapabilityEntry;

template <typename TDomain, typename TCapability>
struct FindCapabilityEntry<TDomain, TCapability> { using Type = void; };

template <typename TDomain, typename TCapability, typename TFirst, typename... TRest>
struct FindCapabilityEntry<TDomain, TCapability, TFirst, TRest...> {
    using Type = std::conditional_t<std::is_same_v<