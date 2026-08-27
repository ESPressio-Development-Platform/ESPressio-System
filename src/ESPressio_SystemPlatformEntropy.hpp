#pragma once

#include <cstddef>
#include <cstdint>

#include "ESPressio_Platform.hpp"

namespace ESPressio {
namespace System {
namespace Entropy {

class IEntropySource {
public:
    virtual ~IEntropySource() = default;

    virtual PlatformResult Fill(void* output, std::size_t size) noexcept = 0;
    virtual bool IsCryptographicallySuitable() const noexcept = 0;
};

class NullEntropySource final : public IEntropySource {
public:
    PlatformResult Fill(void*, std::size_t) noexcept override {
        return PlatformResult::Failed(PlatformStatus::Unavailable);
    }

    bool IsCryptographicallySuitable() const noexcept override {
        return false;
    }
};

inline IEntropySource*& SourceStorage() noexcept {
    static NullEntropySource fallback;
    static IEntropySource* source = &fallback;
    return source;
}

inline IEntropySource& Source() noexcept {
    return *SourceStorage();
}

inline void SetSource(IEntropySource* source) noexcept {
    if (source != nullptr) {
        SourceStorage() = source;
    }
}

inline void ResetSource() noexcept {
    static NullEntropySource fallback;
    SourceStorage() = &fallback;
}

}
}
}
