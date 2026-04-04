// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Sampler"/>.
/// </summary>
public record struct SamplerDescriptor
{
    /// <summary>
    /// Gets or sets the min filter of <see cref="Sampler"/>
    /// </summary>
    public SamplerMinMagFilter MinFilter = SamplerMinMagFilter.Point;

    /// <summary>
    /// Gets or sets the mag filter of <see cref="Sampler"/>
    /// </summary>
    public SamplerMinMagFilter MagFilter = SamplerMinMagFilter.Point;

    /// <summary>
    /// Gets or sets the mip filter of <see cref="Sampler"/>
    /// </summary>
    public SamplerMipFilter MipFilter = SamplerMipFilter.Point;

    /// <summary>
    /// Gets or sets the <see cref="SamplerAddressMode"/> value that specifies the method to use for resolving a u texture coordinate that is outside the 0 to 1 range.
    /// </summary>
    public SamplerAddressMode AddressModeU = SamplerAddressMode.Clamp;

    /// <summary>
    /// Gets or sets the <see cref="SamplerAddressMode"/> value that specifies the method to use for resolving a v texture coordinate that is outside the 0 to 1 range.
    /// </summary>
    public SamplerAddressMode AddressModeV = SamplerAddressMode.Clamp;

    /// <summary>
    /// Gets or sets the <see cref="SamplerAddressMode"/> value that specifies the method to use for resolving a w texture coordinate that is outside the 0 to 1 range.
    /// </summary>
    public SamplerAddressMode AddressModeW = SamplerAddressMode.Clamp;

    /// <summary>
    /// Gets or sets the minimum level of detail (LOD) to use when sampling from a texture.
    /// </summary>
    public float LodMinClamp = 0.0f;

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
    public SamplerReductionType ReductionType = SamplerReductionType.Standard;

    /// <summary>
    /// Gets or sets the <see cref="SamplerBorderColor"/> border color for clamped texture values.
    /// </summary>
    public SamplerBorderColor BorderColor = SamplerBorderColor.FloatTransparentBlack;

    /// <summary>
    /// Gets or sets the label of <see cref="Sampler"/>.
    /// </summary>
    public string? Label;

    public static SamplerDescriptor Default => new();
    public static SamplerDescriptor PointWrap => new(SamplerMinMagFilter.Point, SamplerAddressMode.Wrap);
    public static SamplerDescriptor PointClamp => new(SamplerMinMagFilter.Point, SamplerAddressMode.Clamp);
    public static SamplerDescriptor PointMirror => new(SamplerMinMagFilter.Point, SamplerAddressMode.Mirror);

    public static SamplerDescriptor LinearWrap => new(SamplerMinMagFilter.Linear, SamplerAddressMode.Wrap);
    public static SamplerDescriptor LinearClamp => new(SamplerMinMagFilter.Linear, SamplerAddressMode.Clamp);
    public static SamplerDescriptor LinearMirror => new(SamplerMinMagFilter.Linear, SamplerAddressMode.Mirror);

    public static SamplerDescriptor AnisotropicWrap => new(SamplerMinMagFilter.Point, SamplerAddressMode.Wrap, 16);
    public static SamplerDescriptor AnisotropicClamp => new(SamplerMinMagFilter.Point, SamplerAddressMode.Clamp, 16);
    public static SamplerDescriptor AnisotropicMirror => new(SamplerMinMagFilter.Point, SamplerAddressMode.Mirror, 16);
    public static SamplerDescriptor ComparisonDepth => new(SamplerMinMagFilter.Linear, SamplerAddressMode.Clamp, 1)
    {
        ReductionType = SamplerReductionType.Comparison,
        CompareFunction = CompareFunction.GreaterEqual,
    };


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
    /// <param name="borderColor">Border color to use if <see cref="SamplerAddressMode.Border"/> is specified for AddressU, AddressV, or AddressW.</param>
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
        MipFilter = (filter == SamplerMinMagFilter.Point) ? SamplerMipFilter.Point : SamplerMipFilter.Linear;
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
}
