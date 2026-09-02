#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "ESPressio_Platform.hpp"

namespace ESPressio {
namespace System {
namespace Entropy {

/// <summary>Provides platform entropy bytes and reports whether they are suitable for cryptographic use.</summary>
class IEntropySource {
public:
    virtual ~IEntropySource() = default;

    /// <summary>Fills the supplied buffer with entropy bytes.</summary>
    /// <param name="output">Destination buffer.</param>
    /// <param name="size">Number of bytes requested.</param>
    /// <returns>A platform result describing the outcome.</returns>
    virtual PlatformResult Fill(void* output, std::size_t size) noexcept = 0;

    /// <summary>Indicates whether this source is suitable for cryptographic keying and nonce material.</summary>
    virtual bool IsCryptographicallySuitable() const noexcept = 0;
};

/// <summary>Fallback entropy source that reports entropy services as unavailable.</summary>
class NullEntropySource final : public IEntropySource {
public:
    /// <inheritdoc/>
    PlatformResult Fill(void*, std::size_t) noexcept override {
        return PlatformResult::Failed(PlatformStatus::Unavailable);
    }

    /// <inheritdoc/>
    bool IsCryptographicallySuitable() const noexcept override {
        return false;
    }
};

inline NullEntropySource& FallbackSource() noexcept {
    static NullEntropySource fallback;
    return fallback;
}

inline std::atomic<IEntropySource*>& SourceStorage() noexcept {
    static std::atomic<IEntropySource*> source{&FallbackSource()};
    return source;
}

/// <summary>Gets the active process-wide entropy source.</summary>
inline IEntropySource& Source() noexcept {
    auto* source = SourceStorage().load(std::memory_order_acquire);
    return source != nullptr ? *source : FallbackSource();
}

/// <summary>Installs a non-null process-wide entropy source.</summary>
inline void SetSource(IEntropySource* source) noexcept {
    if (source != nullptr) {
        SourceStorage().store(source, std::memory_order_release);
    }
}

/// <summary>Restores the fallback unavailable entropy source.</summary>
inline void ResetSource() noexcept {
    SourceStorage().store(&FallbackSource(), std::memory_order_release);
}

}
}
}
