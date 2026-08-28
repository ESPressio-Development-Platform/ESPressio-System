#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "ESPressio_Platform.hpp"

namespace ESPressio {
namespace System {
namespace IO {

/// <summary>Provides a byte-oriented input abstraction for platform streams and devices.</summary>
class IByteInput {
public:
    virtual ~IByteInput() = default;

    /// <summary>Gets the number of bytes that can currently be read without waiting.</summary>
    /// <returns>The number of immediately available bytes.</returns>
    virtual std::size_t Available() const noexcept = 0;

    /// <summary>Reads one byte from the input source.</summary>
    /// <param name="value">Receives the byte that was read when the operation succeeds.</param>
    /// <returns>A platform result describing the outcome of the read.</returns>
    virtual PlatformResult Read(uint8_t& value) noexcept = 0;
};

/// <summary>Provides a byte-oriented output abstraction for platform streams and devices.</summary>
class IByteOutput {
public:
    virtual ~IByteOutput() = default;

    /// <summary>Writes a contiguous sequence of bytes to the output destination.</summary>
    /// <param name="data">Pointer to the bytes to write.</param>
    /// <param name="size">Number of bytes requested for writing.</param>
    /// <param name="bytesWritten">Receives the number of bytes actually written.</param>
    /// <returns>A platform result describing the outcome of the write.</returns>
    virtual PlatformResult Write(
        const uint8_t* data,
        std::size_t size,
        std::size_t& bytesWritten
    ) noexcept = 0;

    /// <summary>Writes a single byte.</summary>
    /// <param name="value">The byte to write.</param>
    /// <returns>A platform result describing the outcome of the write.</returns>
    PlatformResult WriteByte(uint8_t value) noexcept {
        std::size_t written = 0;
        return Write(&value, 1, written);
    }

    /// <summary>Writes a null-terminated text string as bytes.</summary>
    /// <param name="text">The null-terminated string to write.</param>
    /// <returns>A platform result describing the outcome of the write.</returns>
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

    /// <summary>Writes optional text followed by a CRLF line terminator.</summary>
    /// <param name="text">Optional null-terminated text to write before the line terminator.</param>
    /// <returns>A platform result describing the outcome of the write.</returns>
    PlatformResult WriteLine(const char* text = nullptr) noexcept {
        if (text != nullptr) {
            const auto textResult = WriteText(text);
            if (!textResult) return textResult;
        }
        return WriteText("\r\n");
    }
};

/// <summary>Combines byte-oriented input and output into a bidirectional stream contract.</summary>
class IByteStream : public IByteInput, public IByteOutput {
public:
    ~IByteStream() override = default;
};

} // namespace IO
} // namespace System
} // namespace ESPressio
