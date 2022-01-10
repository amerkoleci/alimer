// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics;

public enum GpuBackend : byte
{
    Default = 0,
    Vulkan,
    Direct3D12,
    Direct3D11,
}

public enum ValidationMode
{
    /// <summary>
    /// No validation is enabled.
    /// </summary>
    Disabled,
    /// <summary>
    /// Print warnings and errors.
    /// </summary>
    Enabled,
    /// <summary>
    /// Print all warnings, errors and info messages.
    /// </summary>
    Verbose,
    /// Enable GPU-based validation
    GPU
}

public enum GpuPowerPreference
{
    HighPerformance,
    LowPower,
}

public enum GpuAdapterType
{
    DiscreteGPU,
    IntegratedGPU,
    CPU,
    Unknown
}

public enum GpuVendorId : uint
{
    Unknown = 0,
    NVidia = 0x10DE,
    AMD = 0x1002,
    Intel = 0x8086,
    ARM = 0x13B5,
    ImgTec = 0x1010,
    Qualcomm = 0x5143,
    Samsung = 0x1099,
    Microsoft = 0x1414,
}

public enum CommandQueueType
{
    Graphics = 0,
    Compute,
    Copy,
    Count
}

public enum TextureSampleCount
{
    Count1,
    Count2,
    Count4,
    Count8,
    Count16,
    Count32,
}

public enum TextureDimension
{
    Texture1D,
    Texture2D,
    Texture3D,
    TextureCube
}

/// <summary>
/// Defines texture format.
/// </summary>
public enum TextureFormat
{
    Invalid = 0,
    // 8-bit pixel formats
    R8UNorm,
    R8SNorm,
    R8UInt,
    R8SInt,
    // 16-bit pixel formats
    R16UNorm,
    R16SNorm,
    R16UInt,
    R16SInt,
    R16Float,
    RG8UNorm,
    RG8SNorm,
    RG8UInt,
    RG8SInt,
    // 32-bit pixel formats
    R32UInt,
    R32SInt,
    R32Float,
    RG16UNorm,
    RG16SNorm,
    RG16UInt,
    RG16SInt,
    RG16Float,
    RGBA8UNorm,
    RGBA8UNormSrgb,
    RGBA8SNorm,
    RGBA8UInt,
    RGBA8SInt,
    BGRA8UNorm,
    BGRA8UNormSrgb,
    // Packed 32-Bit Pixel formats
    RGB10A2UNorm,
    RG11B10Float,
    RGB9E5Float,
    // 64-Bit Pixel Formats
    RG32UInt,
    RG32SInt,
    RG32Float,
    RGBA16UNorm,
    RGBA16SNorm,
    RGBA16UInt,
    RGBA16SInt,
    RGBA16Float,
    // 128-Bit Pixel Formats
    RGBA32UInt,
    RGBA32SInt,
    RGBA32Float,
    // Depth-stencil formats
    Depth16UNorm,
    Depth32Float,
    Depth24UNormStencil8,
    Depth32FloatStencil8,
    // Compressed BC formats
    BC1RGBAUNorm,
    BC1RGBAUNormSrgb,
    BC2RGBAUNorm,
    BC2RGBAUNormSrgb,
    BC3RGBAUNorm,
    BC3RGBAUNormSrgb,
    BC4RUNorm,
    BC4RSNorm,
    BC5RGUNorm,
    BC5RGSNorm,
    BC6HRGBUFloat,
    BC6HRGBFloat,
    BC7RGBAUNorm,
    BC7RGBAUNormSrgb,

    Count
}

/// <summary>
/// A bitmask indicating how a <see cref="Texture"/> is permitted to be used.
/// </summary>
[Flags]
public enum TextureUsage
{
    None = 0,
    ShaderRead = 1 << 0,
    ShaderWrite = 1 << 1,
    ShaderReadWrite = ShaderRead | ShaderWrite,
    RenderTarget = 1 << 2,
    ShadingRate = 1 << 3,
}
