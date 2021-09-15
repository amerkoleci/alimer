// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "IO/MemoryStream.h"
#include "Core/Log.h"
#include <cstring>

namespace Alimer
{
    MemoryStream::MemoryStream(size_t capacity)
        : capacity{ capacity }
    {
        buffer.resize(capacity);
    }

    MemoryStream::MemoryStream(const std::vector<uint8_t>& data)
    {
        SetData(data);
    }

    MemoryStream::MemoryStream(const void* data, size_t size_)
    {
        SetData(data, size_);
    }

    MemoryStream::MemoryStream(Stream& source, size_t size_)
    {
        SetData(source, size_);
    }

    void MemoryStream::SetData(const std::vector<uint8_t>& data)
    {
        buffer = data;
        position = 0;
        isOpen = true;
        length = data.size();
    }

    void MemoryStream::SetData(const void* data, size_t size_)
    {
        if (!data)
            size_ = 0;

        buffer.resize(size_);
        if (size_)
        {
            memcpy(buffer.data(), data, size_);
        }

        position = 0;
        length = size_;
    }

    void MemoryStream::SetData(Stream& source, size_t size_)
    {
        buffer.resize(size_);
        size_t actualSize = source.Read(&buffer[0], size_);
        if (actualSize != size_)
            buffer.resize(actualSize);

        position = 0;
        size_ = actualSize;
    }

    void MemoryStream::Clear()
    {
        buffer.clear();
        position = 0;
        length = 0;
    }

    void MemoryStream::Resize(size_t newSize)
    {
        buffer.resize(newSize);
        length = newSize;
        if (position > length)
            position = length;
    }

    size_t MemoryStream::Read(void* dest, size_t size_)
    {
        if ((int64_t)size_ + position > length)
            size_ = length - position;
        if (!size_)
            return 0;

        uint8_t* srcPtr = &buffer[position];
        uint8_t* destPtr = (uint8_t*)dest;
        position += size_;

        memcpy(destPtr, srcPtr, size_);

        return size_;
    }

    size_t MemoryStream::Seek(size_t newPosition)
    {
        return position = (newPosition < 0 ? 0 : (newPosition > length ? length : newPosition));
    }

    size_t MemoryStream::Write(const void* data, size_t size_)
    {
        if (!size_)
            return 0;

        if ((int64_t)size_ + position > length)
        {
            length = size_ + position;
            buffer.resize(length);
        }

        uint8_t* srcPtr = (uint8_t*)data;
        uint8_t* destPtr = &buffer[position];
        position += size_;

        memcpy(destPtr, srcPtr, length);

        return size_;
    }

    bool MemoryStream::CanRead() const
    {
        return true;
    }

    bool MemoryStream::CanWrite() const
    {
        return true;
    }

    size_t MemoryStream::Position() const
    {
        return position;
    }

    size_t MemoryStream::Length() const
    {
        return length;
    }
}
