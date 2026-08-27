#include <cassert>
#include <cstddef>
#include <new>
#include <utility>

#include <ESPressio_Memory.hpp>

using namespace ESPressio::System::Memory;

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

int main() {
    TrackingProvider provider;
    IMemoryProvider* previous = SetProvider(&provider);

    {
        Vector<int, MemoryPolicy::ExternalPreferred> values;
        values.push_back(1);
        values.push_back(2);
        assert(values.size() == 2);
        assert(provider.LastPolicy == MemoryPolicy::ExternalPreferred);

        auto object = MakeShared<std::pair<int, int>, MemoryPolicy::ExternalPreferred>(3, 4);
        assert(object->first == 3 && object->second == 4);
        assert(provider.LastPolicy == MemoryPolicy::ExternalPreferred);
    }

    assert(provider.Allocations > 0);
    assert(provider.Deallocations == provider.Allocations);

    SetProvider(previous);
    return 0;
}
