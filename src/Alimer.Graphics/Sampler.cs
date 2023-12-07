// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum SamplerMinMagFilter
{
    Nearest = 0,
    Linear = 1
}

public enum SamplerMipFilter
{
    Nearest = 0,
    Linear = 1
}

public enum SamplerAddressMode
{
    Repeat,
    MirrorRepeat,
    ClampToEdge,
    ClampToBorder,
    MirrorClampToEdge,
}

public enum SamplerBorderColor
{
    FloatTransparentBlack,
    FloatOpaqueBlack,
    FloatOpaqueWhite,
    UintTransparentBlack,
    UintOpaqueBlack,
    UintOpaqueWhite,
}

/// <summary>
/// Structure that describes the <see cref="Sampler"/>.
/// </summary>
public record struct SamplerDescriptor
{
    public static SamplerDescriptor PointWrap => new(SamplerMinMagFilter.Nearest, SamplerAddressMode.Repeat);
    public static SamplerDescriptor PointClamp => new(SamplerMinMagFilter.Nearest, SamplerAddressMode.ClampToEdge);
    public static SamplerDescriptor PointMirror => new(SamplerMinMagFilter.Nearest, SamplerAddressMode.MirrorRepeat);

    public static SamplerDescriptor LinearWrap => new(SamplerMinMagFilter.Linear, SamplerAddressMode.Repeat);
    public static SamplerDescriptor LinearClamp => new(SamplerMinMagFilter.Linear, SamplerAddressMode.ClampToEdge);
    public static SamplerDescriptor LinearMirror => new(SamplerMinMagFilter.Linear, SamplerAddressMode.MirrorRepeat);

    public static SamplerDescriptor AnisotropicWrap => new(SamplerMinMagFilter.Nearest, SamplerAddressMode.Repeat, 16);
    public static SamplerDescriptor AnisotropicClamp => new(SamplerMinMagFilter.Nearest, SamplerAddressMode.ClampToEdge, 16);
    public static SamplerDescriptor AnisotropicMirror => new(SamplerMinMagFilter.Nearest, SamplerAddressMode.MirrorRepeat, 16);

    /// <summary>
    /// Initializes a new instance of the <see cref="SamplerDescriptor"/> struct with default values.
    /// </summary>
    public SamplerDescriptor()
    {

    }

    /// <summary>
    /// Initializes a new instance of the <see cref="SamplerDescriptor"/> struct.
    /// </summary>
    /// <param name="minFilter">The min filter.</param>
    /// <param name="magFilter">The mag filter.</param>
    /// <param name="mipFilter">The mip filter.</param>
    /// <param name="addressModeU">Method to use for resolving a u texture coordinate that is outside the 0 to 1 range.</param>
    /// <param name="addressModeV">Method to use for resolving a v texture coordinate that is outside the 0 to 1 range.</param>
    /// <param name="addressModeW">Method to use for resolving a w texture coordinate that is outside the 0 to 1 range.</param>
    /// <param name="maxAnisotropy">The number of samples that can be taken to improve the quality of sample footprints that are anisotropic. Valid values are between 1 and 16.</param>
    /// <param name="compareFunction">A function that compares sampled data against existing sampled data. </param>
    /// <param name="borderColor">Border color to use if <see cref="TextureAddressMode.Border"/> is specified for AddressU, AddressV, or AddressW.</param>
    /// <param name="lodMinClamp">Lower end of the mipmap range to clamp access to, where 0 is the largest and most detailed mipmap level and any level higher than that is less detailed.</param>
    /// <param name="lodMaxClamp">Upper end of the mipmap range to clamp access to, where 0 is the largest and most detailed mipmap level and any level higher than that is less detailed. This value must be greater than or equal to MinLOD. </param>
    public SamplerDescriptor(
        SamplerMinMagFilter minFilter,
        SamplerMinMagFilter magFilter,
        SamplerMipFilter mipFilter,
        SamplerAddressMode addressModeU,
        SamplerAddressMode addressModeV,
        SamplerAddressMode addressModeW,
        ushort maxAnisotropy = 1,
        CompareFunction compareFunction = CompareFunction.Never,
        float lodMinClamp = 0.0f,
        float lodMaxClamp = float.MaxValue,
        SamplerBorderColor borderColor = SamplerBorderColor.FloatTransparentBlack)
    {
        MinFilter = minFilter;
        MagFilter = magFilter;
        MipFilter = mipFilter;
        AddressModeU = addressModeU;
        AddressModeV = addressModeV;
        AddressModeW = addressModeW;
        MaxAnisotropy = maxAnisotropy;
        CompareFunction = compareFunction;
        LodMinClamp = lodMinClamp;
        LodMaxClamp = lodMaxClamp;
        ReductionType = SamplerReductionType.Standard;
        BorderColor = borderColor;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="SamplerDescriptor"/> struct.
    /// </summary>
    /// <param name="filter">The min, mag and mip filter.</param>
    /// <param name="addressMode">Method to use for resolving a u,v and w texture coordinate that is outside the 0 to 1 range.</param>
    /// <param name="maxAnisotropy">The number of samples that can be taken to improve the quality of sample footprints that are anisotropic. Valid values are between 1 and 16.</param>
    private SamplerDescriptor(SamplerMinMagFilter filter, SamplerAddressMode addressMode, ushort maxAnisotropy = 1)
    {
        MinFilter = filter;
        MagFilter = filter;
        MipFilter = (filter == SamplerMinMagFilter.Nearest) ? SamplerMipFilter.Nearest : SamplerMipFilter.Linear;
        AddressModeU = addressMode;
        AddressModeV = addressMode;
        AddressModeW = addressMode;
        MaxAnisotropy = maxAnisotropy;
        CompareFunction = CompareFunction.Never;
        LodMinClamp = 0.0f;
        LodMaxClamp = float.MaxValue;
        ReductionType = SamplerReductionType.Standard;
        BorderColor = SamplerBorderColor.FloatTransparentBlack;
    }

    /// <summary>
    /// Gets or sets the min filter of <see cref="Sampler"/>
    /// </summary>
    public SamplerMinMagFilter MinFilter = SamplerMinMagFilter.Nearest;

    /// <summary>
    /// Gets or sets the mag filter of <see cref="Sampler"/>
    /// </summary>
    public SamplerMinMagFilter MagFilter = SamplerMinMagFilter.Nearest;

    /// <summary>
    /// Gets or sets the mip filter of <see cref="Sampler"/>
    /// </summary>
    public SamplerMipFilter MipFilter = SamplerMipFilter.Nearest;

    /// <summary>
    /// Gets or sets the <see cref="SamplerAddressMode"/> value that specifies the method to use for resolving a u texture coordinate that is outside the 0 to 1 range.
    /// </summary>
    public SamplerAddressMode AddressModeU = SamplerAddressMode.Repeat;

    /// <summary>
    /// Gets or sets the <see cref="SamplerAddressMode"/> value that specifies the method to use for resolving a v texture coordinate that is outside the 0 to 1 range.
    /// </summary>
    public SamplerAddressMode AddressModeV = SamplerAddressMode.Repeat;

    /// <summary>
    /// Gets or sets the <see cref="SamplerAddressMode"/> value that specifies the method to use for resolving a w texture coordinate that is outside the 0 to 1 range.
    /// </summary>
    public SamplerAddressMode AddressModeW = SamplerAddressMode.Repeat;

    /// <summary>
    /// Gets or sets the minimum level of detail (LOD) to use when sampling from a texture.
    /// </summary>
    public float LodMinClamp { get; init; } = 0.0f;

    /// <summary>
    /// Gets or sets the maximum level of detail (LOD) to use when sampling from a texture.
    /// </summary>
    public float LodMaxClamp = float.MaxValue;

    /// <summary>
    /// Gets or sets the <see cref="Graphics.CompareFunction"/> value that specifies a function that compares sampled data against existing sampled data.
    /// </summary>
    public CompareFunction CompareFunction = CompareFunction.Never;

    /// <summary>
    /// Gets or sets the number of samples that can be taken to improve the quality of sample footprints that are anisotropic.
    /// </summary>
    public ushort MaxAnisotropy = 1;

    /// <summary>
    /// Gets or sets the <see cref="SamplerReductionType"/> value.
    /// </summary>
    public SamplerReductionType ReductionType { get; init; } = SamplerReductionType.Standard;

    /// <summary>
    /// Gets or sets the <see cref="SamplerBorderColor"/> border color for clamped texture values.
    /// </summary>
    public SamplerBorderColor BorderColor = SamplerBorderColor.FloatTransparentBlack;

    /// <summary>
    /// Gets or sets the label of <see cref="Sampler"/>.
    /// </summary>
    public string? Label { get; init; }
}

public abstract class Sampler : GraphicsResource
{
    protected Sampler(in SamplerDescriptor descriptor)
        : base(descriptor.Label)
    {
    }
}
