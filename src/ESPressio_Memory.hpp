#pragma once

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <map>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ESPressio::System::Memory {

enum class MemoryPolicy : unsigned char {
    Automatic = 0,
    Internal,
    ExternalPreferred,
    ExternalRequired
};

class IMemoryProvider {
public:
    virtual ~IMemoryProvider() = default;
    virtual void* Allocate(std::size_t bytes, std::size_t alignment, MemoryPolicy policy) = 0;
    virtual void Deallocate(void* pointer, std::size_t bytes, std::size_t alignment, MemoryPolicy policy) noexcept = 0;
    virtual bool Supports(MemoryPolicy policy) const noexcept = 0;
};

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

inline IMemoryProvider& DefaultProvider() {
    static DefaultMemoryProvider provider;
    return provider;
}

inline std::atomic<IMemoryProvider*>& ProviderSlot() {
    static std::atomic<IMemoryProvider*> provider{&DefaultProvider()};
    return provider;
}

inline IMemoryProvider& GetProvider() noexcept {
    auto* provider = ProviderSlot().load(std::memory_order_acquire);
    return provider ? *provider : DefaultProvider();
}

inline IMemoryProvider* SetProvider(IMemoryProvider* provider) noexcept {
    if (!provider) provider = &DefaultProvider();
    return ProviderSlot().exchange(provider, std::memory_order_acq_rel);
}

inline void ResetProvider() noexcept { (void)SetProvider(&DefaultProvider()); }

template<typename T, MemoryPolicy Policy = MemoryPolicy::Automatic>
class Allocator {
public:
    using value_type = T;
    using is_always_equal = std::false_type;
    template<typename U> struct rebind { using other = Allocator<U, Policy>; };

    Allocator() noexcept : _provider(&GetProvider()) {}
    explicit Allocator(IMemoryProvider& provider) noexcept : _provider(&provider) {}
    template<typename U> Allocator(const Allocator<U, Policy>& other) noexcept : _provider(other.Provider()) {}

    T* allocate(std::size_t count) {
        if (count > static_cast<std::size_t>(-1) / sizeof(T)) throw std::bad_array_new_length();
        return static_cast<T*>(_provider->Allocate(count * sizeof(T), alignof(T), Policy));
    }
    void deallocate(T* pointer, std::size_t count) noexcept {
        _provider->Deallocate(pointer, count * sizeof(T), alignof(T), Policy);
    }
    IMemoryProvider* Provider() const noexcept { return _provider; }

    template<typename U> bool operator==(const Allocator<U, Policy>& other) const noexcept { return _provider == other.Provider(); }
    template<typename U> bool operator!=(const Allocator<U, Policy>& other) const noexcept { return !(*this == other); }
private:
    template<typename, MemoryPolicy> friend class Allocator;
    IMemoryProvider* _provider;
};

template<typename T> using AutomaticAllocator = Allocator<T, MemoryPolicy::Automatic>;
template<typename T> using InternalAllocator = Allocator<T, MemoryPolicy::Internal>;
template<typename T> using ExternalPreferredAllocator = Allocator<T, MemoryPolicy::ExternalPreferred>;
template<typename T> using ExternalRequiredAllocator = Allocator<T, MemoryPolicy::ExternalRequired>;

template<typename T, MemoryPolicy P = MemoryPolicy::Automatic> using Vector = std::vector<T, Allocator<T, P>>;
template<typename T, MemoryPolicy P = MemoryPolicy::Automatic> using Deque = std::deque<T, Allocator<T, P>>;
template<typename K, typename V, MemoryPolicy P = MemoryPolicy::Automatic, typename C = std::less<K>> using Map = std::map<K, V, C, Allocator<std::pair<const K, V>, P>>;
template<typename K, typename V, MemoryPolicy P = MemoryPolicy::Automatic, typename H = std::hash<K>, typename E = std::equal_to<K>> using UnorderedMap = std::unordered_map<K, V, H, E, Allocator<std::pair<const K, V>, P>>;
template<MemoryPolicy P = MemoryPolicy::Automatic> using String = std::basic_string<char, std::char_traits<char>, Allocator<char, P>>;

template<typename T, MemoryPolicy P = MemoryPolicy::Automatic, typename... Args>
std::shared_ptr<T> MakeShared(Args&&... args) {
    return std::allocate_shared<T>(Allocator<T, P>{}, std::forward<Args>(args)...);
}

} // namespace ESPressio::System::Memory
