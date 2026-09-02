#pragma once

#include <memory>
#include <type_traits>
#include <utility>

#include "ESPressio_Memory.hpp"

namespace ESPressio::System::Memory {

/// <summary>Destroys a polymorphic object through the exact derived type and memory provider that created it.</summary>
/// <typeparam name="TBase">Base interface type exposed by the owning pointer.</typeparam>
template<typename TBase>
class PolymorphicObjectDeleter {
public:
    /// <summary>Function used to destroy the derived object and return its storage to the original provider.</summary>
    using DestroyFunction = void (*)(TBase*, IMemoryProvider*) noexcept;

    /// <summary>Creates an inert deleter suitable for a null owning pointer.</summary>
    PolymorphicObjectDeleter() noexcept = default;

    /// <summary>Creates a deleter bound to the provider and derived-type destroy function used for one allocation.</summary>
    PolymorphicObjectDeleter(
        IMemoryProvider& provider,
        DestroyFunction destroy
    ) noexcept : _provider(&provider), _destroy(destroy) {}

    /// <summary>Destroys and releases the supplied polymorphic object.</summary>
    void operator()(TBase* pointer) const noexcept {
        if (pointer == nullptr) return;
        if (_provider != nullptr && _destroy != nullptr) {
            _destroy(pointer, _provider);
        }
    }

    /// <summary>Gets the memory provider that owns the object storage.</summary>
    IMemoryProvider* Provider() const noexcept { return _provider; }

private:
    IMemoryProvider* _provider = nullptr;
    DestroyFunction _destroy = nullptr;
};

/// <summary>Unique ownership of a base-interface pointer whose concrete object storage is policy-aware.</summary>
/// <typeparam name="TBase">Base interface type exposed by the pointer.</typeparam>
template<typename TBase>
using PolymorphicUniquePtr =
    std::unique_ptr<TBase, PolymorphicObjectDeleter<TBase>>;

namespace Detail {

template<typename TBase, typename TDerived, MemoryPolicy P>
void DestroyPolymorphicObject(
    TBase* base,
    IMemoryProvider* provider
) noexcept {
    if (base == nullptr || provider == nullptr) return;
    TDerived* derived = static_cast<TDerived*>(base);
    derived->~TDerived();
    provider->Deallocate(
        derived,
        sizeof(TDerived),
        alignof(TDerived),
        P
    );
}

} // namespace Detail

/// <summary>Constructs a derived object using an ESPressio memory policy and returns unique ownership through a base interface.</summary>
/// <typeparam name="TBase">Base interface type exposed by the returned pointer.</typeparam>
/// <typeparam name="TDerived">Concrete object type to construct.</typeparam>
/// <typeparam name="P">Memory policy used for the concrete object allocation.</typeparam>
/// <param name="args">Arguments forwarded to the concrete object's constructor.</param>
/// <returns>A uniquely owned base-interface pointer whose deleter retains the originating provider and exact derived type.</returns>
template<
    typename TBase,
    typename TDerived,
    MemoryPolicy P = MemoryPolicy::Automatic,
    typename... Args
>
PolymorphicUniquePtr<TBase> MakePolymorphicUnique(Args&&... args) {
    static_assert(
        std::is_base_of_v<TBase, TDerived>,
        "TDerived must derive from TBase"
    );
    static_assert(
        std::is_convertible_v<TDerived*, TBase*>,
        "TDerived must expose an unambiguous, non-private TBase subobject"
    );

    IMemoryProvider& provider = GetProvider();
    void* storage = provider.Allocate(
        sizeof(TDerived),
        alignof(TDerived),
        P
    );

    try {
        TDerived* derived = ::new (storage) TDerived(
            std::forward<Args>(args)...
        );
        TBase* base = static_cast<TBase*>(derived);
        return PolymorphicUniquePtr<TBase>(
            base,
            PolymorphicObjectDeleter<TBase>(
                provider,
                &Detail::DestroyPolymorphicObject<TBase, TDerived, P>
            )
        );
    } catch (...) {
        provider.Deallocate(
            storage,
            sizeof(TDerived),
            alignof(TDerived),
            P
        );
        throw;
    }
}

} // namespace ESPressio::System::Memory
