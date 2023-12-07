// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Defines the type of <see cref="QueueType"/>.
/// </summary>
public enum QueueType
{
    Graphics = 0,
    Compute,
    Copy,
    VideoDecode,
    VideoEncode,

    Count
}
