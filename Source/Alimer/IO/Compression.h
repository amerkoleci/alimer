// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

namespace Alimer
{
    class Stream;
    class MemoryStream;

    /// Estimate and return worst case LZ4 compressed output size in bytes for given input size.
    ALIMER_API uint32_t EstimateCompressBound(uint32_t srcSize);
    /// Compress data using the LZ4 algorithm and return the compressed data size. The needed destination buffer worst-case size is given by EstimateCompressBound().
    ALIMER_API uint32_t CompressData(const void* src, uint32_t srcSize, void* dest, uint32_t compressionLevel = 0);
    /// Uncompress data using the LZ4 algorithm. The uncompressed data size must be known. Return the number of compressed data bytes consumed.
    ALIMER_API uint32_t DecompressData(const void* src, void* dest, uint32_t destSize);
    /// Compress a source stream (from current position to the end) to the destination stream using the LZ4 algorithm.
    ALIMER_API bool CompressStream(Stream& src, Stream& dest);
    /// Decompress a compressed source stream produced using CompressStream() to the destination stream. Return true on success.
    ALIMER_API bool DecompressStream(Stream& src, Stream& dest);
    /// Compress a MemoryStream using the LZ4 algorithm and return the compressed result buffer.
    ALIMER_API MemoryStream CompressMemoryStream(MemoryStream& src);
    /// Decompress a VectorBuffer produced using CompressMemoryStream().
    ALIMER_API MemoryStream DecompressMemoryStream(MemoryStream& src);
}
