#pragma once

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <map>
#include <memory>
#include <new>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ESPressio::System::Memory {

/// <summary>Specifies the memory region preference or requirement for an allocation.</summary>
enum class MemoryPolicy : unsigned char {
    Automatic = 0,
    Internal,
    ExternalPreferred,
    ExternalRequired
};

/// <summary>Abstracts policy-aware allocation and deallocation supplied by the active platform.</summary>
class IMemoryProvider {
public:
    virtual ~IMemoryProvider() = default;

    /// <summary>Allocates a block using the requested size, alignment, and memory policy.</summary>
    /// <param name="bytes">Number of bytes to allocate.</param>
    /// <param name="alignment">Required byte alignment.</param>
    /// <param name="policy">Memory placement policy for the allocation.</param>
    /// <returns>A pointer to the allocated block.</returns>
    virtual void* Allocate(std::size_t bytes, std::size_t alignment, MemoryPolicy policy) = 0;

    /// <summary>Releases a block previously allocated by this provider.</summary>
    virtual void Deallocate(void* pointer, std::size_t bytes, std::size_t alignment, MemoryPolicy policy) noexcept = 0;

    /// <summary>Indicates whether the provider can satisfy the supplied memory policy.</summary>
    virtual bool Supports(MemoryPolicy policy) const noexcept = 0;

    /// <summary>Requests that platform-default/automatic allocations at or above a threshold prefer external memory.</summary>
    /// <param name="minimumBytes">Smallest allocation size that should prefer external memory; zero allows all eligible allocation sizes to prefer external memory.</param>
    /// <returns><c>true</c> when the platform accepted the preference.</returns>
    /// <remarks>
    /// This capability is deliberately optional. It is intended to complement explicit ESPressio allocator policies by
    /// steering third-party and standard-library allocations that use the platform's ordinary default heap. Explicit
    /// <c>Internal</c> allocations and platform capability allocations such as DMA remain governed by their requested
    /// capabilities rather than this preference.
    /// </remarks>
    virtual bool ConfigureAutomaticExternalPreference(std::size_t minimumBytes) noexcept {
        (void)minimumBytes;
        return false;
    }
};

/// <summary>Portable fallback provider backed by the standard C++ allocation operators.</summary>
class DefaultMemoryProvider final : public IMemoryProvider {
public:
    void* Allocate(std::size_t bytes, std::size_t alignment, MemoryPolicy) override {
        if (bytes == 0) bytes = 1;
#if defined(__cpp_aligned_new)
        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) return ::operator new(bytes, std::align_val_t(alignment));
#else
        (void)alignment;
#endif
        return ::operator new(bytes);
    }
    void Deallocate(void* pointer, std::size_t, std::size_t alignment, MemoryPolicy) noexcept override {
        if (!pointer) return;
#if defined(__cpp_aligned_new)
        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) { ::operator delete(pointer, std::align_val_t(alignment)); return; }
#else
        (void)alignment;
#endif
        ::operator delete(pointer);
    }
    bool Supports(MemoryPolicy) const noexcept override { return true; }
};

/// <summary>Gets the portable fallback memory provider.</summary>
inline IMemoryProvider& DefaultProvider() {
    static DefaultMemoryProvider provider;
    return provider;
}

inline std::atomic<IMemoryProvider*>& ProviderSlot() {
    static std::atomic<IMemoryProvider*> provider{&DefaultProvider()};
    return provider;
}

/// <summary>Gets the currently installed process-wide memory provider.</summary>
inline IMemoryProvider& GetProvider() noexcept {
    auto* provider = ProviderSlot().load(std::memory_order_acquire);
    return provider ? *provider : DefaultProvider();
}

/// <summary>Atomically installs a process-wide memory provider.</summary>
/// <param name="provider">Provider to install; null restores the default provider.</param>
/// <returns>The provider that was previously installed.</returns>
inline IMemoryProvider* SetProvider(IMemoryProvider* provider) noexcept {
    if (!provider) provider = &DefaultProvider();
    return ProviderSlot().exchange(provider, std::memory_order_acq_rel);
}

/// <summary>Restores the portable fallback memory provider.</summary>
inline void ResetProvider() noexcept { (void)SetProvider(&DefaultProvider()); }

/// <summary>Configures the active platform's ordinary automatic-allocation external-memory preference when supported.</summary>
/// <param name="minimumBytes">Smallest ordinary allocation that should prefer external memory; zero allows every eligible size.</param>
/// <returns><c>true</c> when the active provider supports and accepted the preference.</returns>
/// <remarks>
/// Use this only after the platform memory provider has been installed. Explicit ESPressio memory policies continue to
/// take precedence. The setting is useful for large third-party/STL allocations that cannot directly consume an
/// ESPressio allocator, while capability-specific platform allocations remain unaffected.
/// </remarks>
inline bool ConfigureAutomaticExternalPreference(std::size_t minimumBytes) noexcept {
    return GetProvider().ConfigureAutomaticExternalPreference(minimumBytes);
}

/// <summary>Standard-library-compatible allocator that routes storage through an ESPressio memory provider.</summary>
/// <typeparam name="T">Element type allocated by the allocator.</typeparam>
/// <typeparam name="Policy">Memory placement policy applied to every allocation.</typeparam>
/// <remarks>
/// Default-constructed allocators bind lazily to the active provider on their first allocation. This is important for
/// ESPressio objects created during static initialization: their containers can be constructed before a platform provider
/// is installed without becoming permanently bound to the portable fallback heap. Once an allocator performs an
/// allocation it retains that provider so the matching deallocation always returns storage to the correct heap.
/// </remarks>
template<typename T, MemoryPolicy Policy = MemoryPolicy::Automatic>
class Allocator {
public:
    using value_type = T;
    using is_always_equal = std::false_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    template<typename U> struct rebind { using other = Allocator<U, Policy>; };

    /// <summary>Creates an allocator that binds to the active provider on its first allocation.</summary>
    Allocator() noexcept = default;
    /// <summary>Creates an allocator bound to an explicit memory provider.</summary>
    explicit Allocator(IMemoryProvider& provider) noexcept : _provider(&provider) {}
    /// <summary>Copies the provider binding from a compatible allocator without forcing an unbound allocator to bind early.</summary>
    template<typename U> Allocator(const Allocator<U, Policy>& other) noexcept : _provider(other._provider) {}

    /// <summary>Allocates storage for the requested number of elements.</summary>
    T* allocate(std::size_t count) {
        if (count > static_cast<std::size_t>(-1) / sizeof(T)) throw std::bad_array_new_length();
        IMemoryProvider* provider = BindProvider();
        return static_cast<T*>(provider->Allocate(count * sizeof(T), alignof(T), Policy));
    }
    /// <summary>Releases storage previously allocated for the supplied element count.</summary>
    void deallocate(T* pointer, std::size_t count) noexcept {
        if (pointer == nullptr) return;
        IMemoryProvider* provider = _provider != nullptr ? _provider : &GetProvider();
        provider->Deallocate(pointer, count * sizeof(T), alignof(T), Policy);
    }
    /// <summary>Gets the effective provider, resolving an as-yet-unbound allocator against the currently active provider.</summary>
    IMemoryProvider* Provider() const noexcept { return _provider != nullptr ? _provider : &GetProvider(); }
    /// <summary>Indicates whether this allocator has already committed to a provider by allocating or explicit construction.</summary>
    bool IsProviderBound() const noexcept { return _provider != nullptr; }

    template<typename U> bool operator==(const Allocator<U, Policy>& other) const noexcept { return Provider() == other.Provider(); }
    template<typename U> bool operator!=(const Allocator<U, Policy>& other) const noexcept { return !(*this == other); }
private:
    IMemoryProvider* BindProvider() noexcept {
        if (_provider == nullptr) _provider = &GetProvider();
        return _provider;
    }

    template<typename, MemoryPolicy> friend class Allocator;
    IMemoryProvider* _provider = nullptr;
};

template<typename T> using AutomaticAllocator = Allocator<T, MemoryPolicy::Automatic>;
template<typename T> using InternalAllocator = Allocator<T, MemoryPolicy::Internal>;
template<typename T> using ExternalPreferredAllocator = Allocator<T, MemoryPolicy::ExternalPreferred>;
template<typename T> using ExternalRequiredAllocator = Allocator<T, MemoryPolicy::ExternalRequired>;

template<typename T, MemoryPolicy P = MemoryPolicy::Automatic> using Vector = std::vector<T, Allocator<T, P>>;
template<typename T, MemoryPolicy P = MemoryPolicy::Automatic> using Deque = std::deque<T, Allocator<T, P>>;
template<typename K, typename V, MemoryPolicy P = MemoryPolicy::Automatic, typename C = std::less<K>> using Map = std::map<K, V, C, Allocator<std::pair<const K, V>, P>>;
template<typename K, typename V, MemoryPolicy P = MemoryPolicy::Automatic, typename H = std::hash<K>, typename E = std::equal_to<K>> using UnorderedMap = std::unordered_map<K, V, H, E, Allocator<std::pair<const K, V>, P>>;
template<typename T, MemoryPolicy P = MemoryPolicy::Automatic, typename C = std::less<T>> using Set = std::set<T, C, Allocator<T, P>>;
template<typename T, MemoryPolicy P = MemoryPolicy::Automatic, typename H = std::hash<T>, typename E = std::equal_to<T>> using UnorderedSet = std::unordered_set<T, H, E, Allocator<T, P>>;
template<MemoryPolicy P = MemoryPolicy::Automatic> using String = std::basic_string<char, std::char_traits<char>, Allocator<char, P>>;
template<MemoryPolicy P = MemoryPolicy::Automatic> using ByteVector = Vector<unsigned char, P>;

/// <summary>Destroys and releases one object through the provider and policy that allocated it.</summary>
/// <typeparam name="T">Object type owned by the deleter.</typeparam>
/// <typeparam name="P">Memory policy used for the object allocation.</typeparam>
template<typename T, MemoryPolicy P = MemoryPolicy::Automatic>
class ObjectDeleter {
public:
    /// <summary>Creates a deleter bound to the currently installed memory provider.</summary>
    ObjectDeleter() noexcept : _provider(&GetProvider()) {}

    /// <summary>Creates a deleter bound to the supplied memory provider.</summary>
    explicit ObjectDeleter(IMemoryProvider& provider) noexcept : _provider(&provider) {}

    /// <summary>Destroys and releases the supplied object.</summary>
    void operator()(T* pointer) const noexcept {
        if (pointer == nullptr) return;
        pointer->~T();
        _provider->Deallocate(pointer, sizeof(T), alignof(T), P);
    }

    /// <summary>Gets the memory provider used by this deleter.</summary>
    IMemoryProvider* Provider() const noexcept { return _provider; }

private:
    IMemoryProvider* _provider;
};

/// <summary>Unique ownership whose object storage is allocated by an ESPressio memory provider.</summary>
template<typename T, MemoryPolicy P = MemoryPolicy::Automatic>
using UniquePtr = std::unique_ptr<T, ObjectDeleter<T, P>>;

/// <summary>Constructs a uniquely owned object using the ESPressio allocator and selected memory policy.</summary>
/// <typeparam name="T">Object type to construct.</typeparam>
/// <typeparam name="P">Memory policy used for the object allocation.</typeparam>
template<typename T, MemoryPolicy P = MemoryPolicy::Automatic, typename... Args>
UniquePtr<T, P> MakeUnique(Args&&... args) {
    static_assert(!std::is_array_v<T>, "ESPressio MakeUnique currently supports object types, not arrays.");
    IMemoryProvider& provider = GetProvider();
    void* storage = provider.Allocate(sizeof(T), alignof(T), P);
    try {
        T* object = ::new (storage) T(std::forward<Args>(args)...);
        return UniquePtr<T, P>(object, ObjectDeleter<T, P>(provider));
    } catch (...) {
        provider.Deallocate(storage, sizeof(T), alignof(T), P);
        throw;
    }
}

/// <summary>Shared ownership type used by ESPressio components.</summary>
/// <remarks>Construct shared objects with MakeShared so both object and control-block storage use the ESPressio allocator.</remarks>
template<typename T>
using SharedPtr = std::shared_ptr<T>;

/// <summary>Constructs a shared object using the ESPressio allocator and selected memory policy.</summary>
/// <typeparam name="T">Object type to construct.</typeparam>
/// <typeparam name="P">Memory policy used for the shared allocation.</typeparam>
template<typename T, MemoryPolicy P = MemoryPolicy::Automatic, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return std::allocate_shared<T>(Allocator<T, P>{}, std::forward<Args>(args)...);
}

} // namespace ESPressio::System::Memory