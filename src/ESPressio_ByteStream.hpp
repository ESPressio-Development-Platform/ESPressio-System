#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "ESPressio_Platform.hpp"

namespace ESPressio {
namespace System {
namespace IO {

class IByteInput {
public:
    virtual ~IByteInput() = default;

    virtual std::size_t Available() const noexcept = 0;
    virtual PlatformResult Read(uint8_t& value) noexcept = 0;
};

class IByteOutput {
public:
    virtual ~IByteOutput() = default;

    virtual PlatformResult Write(
        const uint8_t* data,
        std::size_t size,
        std::size_t& bytesWritten
    ) noexcept = 0;

    PlatformResult WriteByte(uint8_t value) noexcept {
        std::size_t written = 0;
        return Write(&value, 1, written);
    }

    PlatformResult WriteText(const char* text) noexcept {
        if (text == nullptr) {
            return PlatformResult::Failed(PlatformStatus::InvalidArgument);
        }
        std::size_t written = 0;
        return Write(
            reinterpret_cast<const uint8_t*>(text),
            std::strlen(text),
            written
        );
    }

    PlatformResult WriteLine(const char* text = nullptr) noexcept {
        if (text != nullptr) {
            const auto textResult = WriteText(text);
            if (!textResult) return textResult;
        }
        return WriteText("\r\n");
    }
};

class IByteStream : public IByteInput, public IByteOutput {
public:
    ~IByteStream() override = default;
};

} // namespace IO
} // namespace System
} // namespace ESPressio
