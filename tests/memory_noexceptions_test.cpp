#include <cstddef>

#include "ESPressio_Memory.hpp"

using namespace ESPressio::System::Memory;

class FixedProvider final : public IMemoryProvider {
    alignas(std::max_align_t) unsigned char storage_[128]{};
    bool used_{false};
public:
    void* Allocate(std::size_t bytes, std::size_t alignment, MemoryPolicy) override {
        if (used_ || bytes > sizeof(storage_) || alignment > alignof(std::max_align_t)) std::abort();
        used_ = true;
        return storage_;
    }

    void Deallocate(void* pointer, std::size_t, std::size_t, MemoryPolicy) noexcept override {
        if (pointer == storage_) used_ = false;
    }

    bool Supports(MemoryPolicy) const noexcept override { return true; }
};

int main() {
    FixedProvider provider;
    IMemoryProvider* previous = SetProvider(&provider);
    {
        Vector<int, MemoryPolicy::ExternalPreferred> values;
        values.reserve(4U);
        values.push_back(7);
        values.push_back(11);
        if (values.size() != 2U || values[0] != 7 || values[1] != 11) return 1;
    }
    SetProvider(previous);
    return 0;
}
