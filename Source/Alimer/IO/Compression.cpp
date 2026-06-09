// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/IO/Compression.h"
#include "Alimer/IO/MemoryStream.h"
#include <lz4.h>
#include <lz4hc.h>

namespace Alimer
{
    uint32_t EstimateCompressBound(uint32_t srcSize)
    {
        return (uint32_t)LZ4_compressBound(srcSize);
    }

    uint32_t CompressData(const void* src, uint32_t srcSize, void* dest, uint32_t compressionLevel)
    {
        if (!dest || !src || !srcSize)
            return 0;

        //return (unsigned)LZ4_compress_default((const char*)src, (char*)dest, srcSize, LZ4_compressBound(srcSize));
        return (uint32_t)LZ4_compress_HC((const char*)src, (char*)dest, srcSize, LZ4_compressBound(srcSize), static_cast<int>(compressionLevel));
    }

    uint32_t DecompressData(const void* src, void* dest, uint32_t destSize)
    {
        if (!dest || !src || !destSize)
            return 0;

        return (uint32_t)LZ4_decompress_safe((const char*)src, (char*)dest, (int)destSize, LZ4_compressBound(destSize));
    }

    bool CompressStream(Stream& src, Stream& dest)
    {
        const size_t srcSize = src.GetSize() - src.GetPosition();
        // Prepend the source and dest. data size in the stream so that we know to buffer & uncompress the right amount
        if (!srcSize)
        {
            dest.Write(0u);
            dest.Write(0u);
            return false;
        }

        const uint32_t maxDestSize = (uint32_t)LZ4_compressBound((int)srcSize);
        std::unique_ptr<uint8_t> srcBuffer(new uint8_t[srcSize]);
        std::unique_ptr<uint8_t> destBuffer(new uint8_t[maxDestSize]);

        if (src.Read(srcBuffer.get(), srcSize) != srcSize)
            return false;

        uint32_t destSize = (uint32_t)LZ4_compress_HC((const char*)srcBuffer.get(), (char*)destBuffer.get(), (int)srcSize, LZ4_compressBound((int)srcSize), 0);
        dest.Write((uint32_t)srcSize);
        dest.Write(destSize);
        dest.Write(destBuffer.get(), destSize);
        return true;
    }

    bool DecompressStream(Stream& src, Stream& dest)
    {
        if (src.IsEof())
            return false;

        const uint32_t destSize = src.ReadUInt32();
        const uint32_t srcSize = src.ReadUInt32();
        if (!srcSize || !destSize)
            return true; // No data

        if (srcSize > src.GetSize())
            return false; // Illegal source (packed data) size reported, possibly not valid data

        std::unique_ptr<uint8_t> srcBuffer(new uint8_t[srcSize]);
        std::unique_ptr<uint8_t> destBuffer(new uint8_t[destSize]);

        if (src.Read(srcBuffer.get(), srcSize) != srcSize)
            return false;

        LZ4_decompress_safe((const char*)srcBuffer.get(), (char*)destBuffer.get(), destSize, LZ4_compressBound(destSize));
        return dest.Write(destBuffer.get(), destSize) == destSize;
    }

    MemoryStream CompressMemoryStream(MemoryStream& src)
    {
        MemoryStream result;
        src.Seek(0);
        CompressStream(result, src);
        result.Seek(0);
        return result;
    }

    MemoryStream DecompressMemoryStream(MemoryStream& src)
    {
        MemoryStream result;
        src.Seek(0);
        DecompressStream(result, src);
        result.Seek(0);
        return result;
    }
}
