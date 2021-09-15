// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "IO/Stream.h"

namespace Alimer
{
    /// Memory area that can be read and written to as a stream.
    class ALIMER_API MemoryStream : public Stream
    {
    public:
        /// Construct an empty stream.
        MemoryStream() = default;
        /// Construct an empty stream with capacity.
        MemoryStream(size_t capacity);
        /// Construct from another buffer.
        MemoryStream(const std::vector<uint8_t>& data);
        /// Construct as read-only with a pointer and size.
        MemoryStream(const void* data, size_t size_);
        /// Construct from another stream.
        MemoryStream(Stream& source, size_t size_);

        /// Set data from another buffer.
        void SetData(const std::vector<uint8_t>& data);
        /// Set data from a memory area.
        void SetData(const void* data, size_t size_);
        /// Set data from a stream.
        void SetData(Stream& source, size_t size_);
        /// Reset to zero size.
        void Clear();
        /// Set size.
        void Resize(size_t newSize);

        /// Read bytes from the memory area. Return number of bytes actually read.
        size_t Read(void* dest, size_t size) override;
        /// Set position in bytes from the beginning of the memory area.
        size_t Seek(size_t position) override;
        /// Write bytes to the memory area.
        size_t Write(const void* data, size_t size) override;
        /// Return whether read operations are allowed.
        bool CanRead() const override;
        /// Return whether write operations are allowed.
        bool CanWrite() const override;
        /// Return current position in bytes.
        size_t Position() const override;
        /// Return the length in bytes of the stream.
        size_t Length() const override;

        uint8_t* Data() { return buffer.data(); }
        const uint8_t* Data() const { return buffer.data(); }

        /// Return the buffer.
        const std::vector<uint8_t>& GetBuffer() const { return buffer; }
        /// Return the buffer.
        std::vector<uint8_t>& GetBuffer() { return buffer; }

        using Stream::Read;
        using Stream::Write;

    private:
        size_t position{ 0 };
        size_t length{ 0 };
        size_t capacity{ 0 };

        bool expandable{ true };
        bool writable{ true };
        bool exposable{ true };
        bool isOpen{ true };

        /// Pointer to the memory area.
        std::vector<uint8_t> buffer;
    };
}
