﻿// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;

namespace Alimer.Graphics.D3D12;

internal static unsafe class PixHelpers
{
    public const uint WinPIXEventPIX3BlobVersion = 2;
    private const ulong PIXEventsBlockEndMarker = 0xFFF80;

    public enum PixEventType
    {
        PIXEvent_EndEvent = 0x000,
        PIXEvent_BeginEvent_VarArgs = 0x001,
        PIXEvent_BeginEvent_NoArgs = 0x002,
        PIXEvent_SetMarker_VarArgs = 0x007,
        PIXEvent_SetMarker_NoArgs = 0x008,
    }

    /// <summary>
    /// Calculates the size of the buffer required, in bytes.
    /// </summary>
    /// <param name="message"> The text to be embedded in the event. </param>
    /// <returns> The size of the buffer required, in bytes. </returns>
    internal static int CalculateNoArgsEventSize(ReadOnlySpan<char> message) =>
        (3 /* start marker, color, copy chunk size */ +
        message.Length / 4 /* 8 bytes / 2 bytes per character */ +
        1 /* null terminator */ +
        1 /* end marker */) * 8;

    /// <summary>
    /// Writes a formatted PIX no-arg event to the given buffer.
    /// </summary>
    /// <param name="outputBuffer"> The buffer to write to. </param>
    /// <param name="eventType"> The PIX event type. </param>
    /// <param name="color"> Either a color index, or an RGB color. </param>
    /// <param name="message"> The message to embed in the buffer. </param>
    internal static unsafe void FormatNoArgsEventToBuffer(void* outputBuffer, PixEventType eventType, ulong color, ReadOnlySpan<char> message)
    {
        // The buffer is written to in quad-word-sized chunks.
        ulong* buffer = (ulong*)outputBuffer;

        // The first word contains the event type and the timestamp.
        // Bits 10-19 (10 bits) for the event type.
        // Bits 20-63 (44 bits) for timestamp (as produced by QueryPerformanceCounter).
        var timestamp = (ulong)Stopwatch.GetTimestamp();
        buffer[0] = ((timestamp & 0x00000FFFFFFFFFFF) << 20) |
                    (((ulong)eventType & 0x00000000000003FF) << 10);

        // 0xff000000 | (r << 16) | (g << 8) | b
        // or: color index (byte)i
        buffer[1] = color;

        // Bit 53: isShortcut
        // Bit 54: isANSI
        // Bit 55-59: copyChunkSize
        // Bits 60-63: alignment
        buffer[2] = (8UL /* copyChunkSize */ & 0x1F) << 55;

        // Write message, with a null terminator.
        int strIndex = 0, bufferIndex = 3;
        while (true)
        {
            // char #1
            if (strIndex >= message.Length)
            {
                buffer[bufferIndex++] = 0;
                break;
            }
            uint c = message[strIndex++];
            ulong longValue = c;

            // char #2
            if (strIndex >= message.Length)
            {
                buffer[bufferIndex++] = longValue;
                break;
            }
            c = message[strIndex++];
            longValue |= (ulong)c << 16;

            // char #3
            if (strIndex >= message.Length)
            {
                buffer[bufferIndex++] = longValue;
                break;
            }
            c = message[strIndex++];
            longValue |= (ulong)c << 32;

            // char #4
            if (strIndex >= message.Length)
            {
                buffer[bufferIndex++] = longValue;
                break;
            }
            c = message[strIndex++];
            longValue |= (ulong)c << 48;

            // Write to the buffer.
            buffer[bufferIndex++] = longValue;
        }

        buffer[bufferIndex] = PIXEventsBlockEndMarker;
    }
}
