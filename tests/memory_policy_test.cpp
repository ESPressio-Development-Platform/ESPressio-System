#include <cassert>
#include <cstddef>
#include <new>
#include <set>
#include <unordered_set>
#include <utility>

#include <ESPressio_Memory.hpp>
#include <ESPressio_PolymorphicMemory.hpp>

using namespace ESPressio::System::Memory;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members:
 * - Allocations (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * - Deallocations (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * - LastPolicy (MemoryPolicy): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 16 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class TrackingProvider final : public IMemoryProvider {
public:
    std::size_t Allocations = 0;
    std::size_t Deallocations = 0;
    MemoryPolicy LastPolicy = MemoryPolicy::Automatic;

    void* Allocate(std::size_t bytes, std::size_t, MemoryPolicy policy) override {
        ++Allocations;
        LastPolicy = policy;
        return ::operator new(bytes == 0 ? 1 : bytes);
    }

    void Deallocate(void* pointer, std::size_t, std::size_t, MemoryPolicy policy) noexcept override {
        ++Deallocations;
        LastPolicy = policy;
        ::operator delete(pointer);
    }

    bool Supports(MemoryPolicy) const noexcept override { return true; }
};

/**
 * ESPressio Memory Audit
 * Members:
 * - Value (int): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct TrackedObject {
    static int Alive;
    int Value = 0;

    explicit TrackedObject(int value) : Value(value) { ++Alive; }
    ~TrackedObject() { --Alive; }
};

int TrackedObject::Alive = 0;

/**
 * ESPressio Memory Audit
 * Members: none; polymorphic/virtual-base object metadata is included in the total.
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct PolymorphicBase {
    virtual ~PolymorphicBase() = default;
    virtual int Value() const = 0;
};

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members:
 * - Stored (int): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct PolymorphicDerived final : PolymorphicBase {
    static int Alive;
    int Stored = 0;

    explicit PolymorphicDerived(int value) : Stored(value) { ++Alive; }
    ~PolymorphicDerived() override { --Alive; }
    int Value() const override { return Stored; }
};

int PolymorphicDerived::Alive = 0;

int main() {
    // Model an ESPressio singleton/global whose allocator-aware member is
    // constructed before the platform provider is installed. The allocator
    // must bind only when storage is actually requested.
    Vector<int, MemoryPolicy::ExternalPreferred> lateBoundValues;
    assert(!lateBoundValues.get_allocator().IsProviderBound());

    TrackingProvider provider;
    IMemoryProvider* previous = SetProvider(&provider);

    {
        lateBoundValues.push_back(9);
        assert(lateBoundValues.size() == 1);
        assert(provider.Allocations == 1);
        assert(provider.LastPolicy == MemoryPolicy::ExternalPreferred);
        assert(lateBoundValues.get_allocator().IsProviderBound());

        Vector<int, MemoryPolicy::ExternalPreferred> values;
        values.push_back(1);
        values.push_back(2);
        assert(values.size() == 2);
        assert(provider.LastPolicy == MemoryPolicy::ExternalPreferred);

        ByteVector<MemoryPolicy::ExternalPreferred> bytes;
        bytes.push_back(0x12);
        bytes.push_back(0x34);
        assert(bytes.size() == 2);
        assert(provider.LastPolicy == MemoryPolicy::ExternalPreferred);

        Set<int, MemoryPolicy::ExternalPreferred> ordered;
        ordered.insert(3);
        assert(ordered.count(3) == 1);
        assert(provider.LastPolicy == MemoryPolicy::ExternalPreferred);

        UnorderedSet<int, MemoryPolicy::ExternalPreferred> unordered;
        unordered.insert(4);
        assert(unordered.count(4) == 1);
        assert(provider.LastPolicy == MemoryPolicy::ExternalPreferred);

        auto object = MakeShared<std::pair<int, int>, MemoryPolicy::ExternalPreferred>(3, 4);
        assert(object->first == 3 && object->second == 4);
        assert(provider.LastPolicy == MemoryPolicy::ExternalPreferred);

        auto unique = MakeUnique<TrackedObject, MemoryPolicy::ExternalPreferred>(7);
        assert(unique);
        assert(unique->Value == 7);
        assert(TrackedObject::Alive == 1);
        assert(provider.LastPolicy == MemoryPolicy::ExternalPreferred);

        auto polymorphic = MakePolymorphicUnique<
            PolymorphicBase,
            PolymorphicDerived,
            MemoryPolicy::ExternalPreferred
        >(11);
        assert(polymorphic);
        assert(polymorphic->Value() == 11);
        assert(PolymorphicDerived::Alive == 1);
        assert(provider.LastPolicy == MemoryPolicy::ExternalPreferred);

        TrackingProvider replacement;
        SetProvider(&replacement);
        unique.reset();
        polymorphic.reset();
        assert(TrackedObject::Alive == 0);
        assert(PolymorphicDerived::Alive == 0);
        assert(provider.Deallocations > 0);
        assert(replacement.Deallocations == 0);
        SetProvider(&provider);
    }

    // Releasing a container after the active provider changes must still use
    // the provider that supplied its storage.
    const std::size_t deallocationsBefore = provider.Deallocations;
    TrackingProvider replacement;
    SetProvider(&replacement);
    lateBoundValues.clear();
    lateBoundValues.shrink_to_fit();
    assert(provider.Deallocations > deallocationsBefore);
    assert(replacement.Deallocations == 0);
    SetProvider(&provider);

    assert(provider.Allocations > 0);
    assert(provider.Deallocations == provider.Allocations);

    SetProvider(previous);
    return 0;
}
