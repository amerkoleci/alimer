// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public static class Constants
{
    public const int DefaultMaxFramesInFlight = 2;

    public const int MaxColorAttachments = 8;
    public const int MaxVertexBufferBindings = 8;
    public const int MaxVertexAttributes = 16;
    public const int MaxVertexAttributeOffset = 2047;
    public const int MaxVertexBufferStride = 2048;
    public const int MaxMipLevels = 16;
    public const int MaxSamplerAnisotropy = 16;

    public const int QuerySetMaxQueries = 8192;

    public const uint MipLevelCountUndefined = uint.MaxValue;
    public const uint ArrayLayerCountUndefined = uint.MaxValue;
    public const ulong WholeSize = ulong.MaxValue;

    // Must match the static sampler Alimer.hlsl
    public const int DynamicContantBufferCount = 4; // b0, b1, b2, b3 in shader
    public const int StaticSamplerCount = 10; 
    public const int StaticSamplerRegisterSpaceBegin = 100;

    /// <summary>
    /// Invalid bindless index.
    /// </summary>
    public const int InvalidBindlessIndex = -1;

    /// <summary>
    /// Bindless resource capacity, the device can allocate less than this, depending on capabilities.
    /// </summary>
    public const int BindlessResourceCapacity = 500000;

    /// <summary>
    /// Bindless sampler capacity, the device can allocate less than this, depending on capabilities.
    /// </summary>
    public const int BindlessSamplerCapacity = 256;

    public const int PushConstantsSize = 128; // 128 bytes is the minimum guaranteed size for push constants in Vulkan
}
