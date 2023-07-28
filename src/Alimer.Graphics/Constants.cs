// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public static class Constants
{
    public const uint MaxFramesInFlight = 2u;

    public const int MaxBindGroups = 4;
    public const int MaxColorAttachments = 8;
    public const int MaxVertexBufferBindings = 8;
    public const int MaxVertexAttributes = 16;
    public const int MaxVertexAttributeOffset = 2047;
    public const int MaxVertexBufferStride = 2048;
    public const int MaxMipLevels = 16;
    public const int MaxSamplerAnisotropy = 16;
    public const int MaxPushConstantsSize = 256;

    public const int QuerySetMaxQueries = 8192;
}
