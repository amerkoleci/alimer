// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Texture"/>.
/// </summary>
public record struct TextureDescriptor
{
    [SetsRequiredMembers]
    public TextureDescriptor(
        TextureDimension dimension,
        PixelFormat format,
        uint width,
        uint height,
        uint depthOrArrayLayers,
        uint mipLevelCount = 1u,
        TextureUsage usage = TextureUsage.ShaderRead,
        TextureSampleCount sampleCount = TextureSampleCount.Count1,
        CpuAccessMode access = CpuAccessMode.None,
        ResourceStates initialLayout = ResourceStates.ShaderResource,
        string? label = default)
    {
        Guard.IsTrue(format != PixelFormat.Undefined);
        Guard.IsGreaterThanOrEqualTo(width, 1);
        Guard.IsGreaterThanOrEqualTo(height, 1);
        Guard.IsGreaterThanOrEqualTo(depthOrArrayLayers, 1);

        Dimension = dimension;
        Format = format;
        Width = width;
        Height = height;
        DepthOrArrayLayers = depthOrArrayLayers;
        MipLevelCount = mipLevelCount == 0 ? ImageDescription.GetMipLevelCount(width, height, dimension == TextureDimension.Texture3D ? depthOrArrayLayers : 1u) : mipLevelCount;
        SampleCount = sampleCount;
        Usage = usage;
        CpuAccess = access;
        InitialLayout = initialLayout;
        Label = label;
    }

    public static TextureDescriptor Texture1D(
        PixelFormat format,
        uint width,
        uint mipLevels = 1,
        uint arrayLayers = 1u,
        TextureUsage usage = TextureUsage.ShaderRead,
        CpuAccessMode access = CpuAccessMode.None,
        ResourceStates initialLayout = ResourceStates.ShaderResource,
        string? label = default)
    {
        return new(
            TextureDimension.Texture1D,
            format,
            width,
            1,
            arrayLayers,
            mipLevels,
            usage,
            TextureSampleCount.Count1,
            access,
            initialLayout,
            label);
    }

    public static TextureDescriptor Texture2D(
        PixelFormat format,
        uint width,
        uint height,
        uint mipLevels = 1u,
        uint arrayLayers = 1u,
        TextureUsage usage = TextureUsage.ShaderRead,
        TextureSampleCount sampleCount = TextureSampleCount.Count1,
        CpuAccessMode access = CpuAccessMode.None,
        ResourceStates initialLayout = ResourceStates.ShaderResource,
        string? label = default)
    {
        return new(
            TextureDimension.Texture2D,
            format,
            width,
            height,
            arrayLayers,
            mipLevels,
            usage,
            sampleCount,
            access,
            initialLayout,
            label);
    }

    public static TextureDescriptor Texture3D(
        PixelFormat format,
        uint width,
        uint height,
        uint depth,
        uint mipLevels = 1u,
        TextureUsage usage = TextureUsage.ShaderRead,
        CpuAccessMode access = CpuAccessMode.None,
        ResourceStates initialLayout = ResourceStates.ShaderResource,
        string? label = default)
    {
        return new(
            TextureDimension.Texture3D,
            format,
            width,
            height,
            depth,
            mipLevels,
            usage,
            TextureSampleCount.Count1,
            access,
            initialLayout,
            label);
    }

    public static TextureDescriptor TextureCube(
        PixelFormat format,
        uint size,
        uint mipLevels = 1u,
        uint arrayLayers = 1u,
        TextureUsage usage = TextureUsage.ShaderRead,
        CpuAccessMode access = CpuAccessMode.None,
        ResourceStates initialLayout = ResourceStates.ShaderResource,
        string? label = default)
    {
        return new(
            TextureDimension.Texture2D,
            format,
            size,
            size,
            arrayLayers * 6,
            mipLevels,
            usage,
            TextureSampleCount.Count1,
            access,
            initialLayout,
            label);
    }

    /// <summary>
    /// Gets the dimension of <see cref="Texture"/>
    /// </summary>
    public required TextureDimension Dimension { get; init; }

    /// <summary>
    /// Gets the pixel format of <see cref="Texture"/>
    /// </summary>
    public required PixelFormat Format { get; init; }

    /// <summary>
    /// Gets the width of <see cref="Texture"/>
    /// </summary>
    public required uint Width { get; init; } = 1;

    /// <summary>
    /// Gets the height of <see cref="Texture"/>
    /// </summary>
    public required uint Height { get; init; } = 1;

    /// <summary>
    /// Gets the depth of <see cref="Texture"/>, if it is 3D, or the array layers if it is an array of 1D or 2D resources.
    /// </summary>
    public required uint DepthOrArrayLayers { get; init; } = 1;

    /// <summary>
    /// The number of mipmap levels in the <see cref="Texture"/>.
    /// </summary>
    public required uint MipLevelCount { get; init; } = 1;

    /// <summary>
    /// Gets the <see cref="TextureUsage"/> of <see cref="Texture"/>.
    /// </summary>
    public TextureUsage Usage { get; init; } = TextureUsage.ShaderRead;

    /// <summary>
    /// Gets the texture sample count.
    /// </summary>
    public TextureSampleCount SampleCount { get; init; } = TextureSampleCount.Count1;

    /// <summary>
    /// CPU access of the <see cref="Texture"/>.
    /// </summary>
    public CpuAccessMode CpuAccess { get; init; } = CpuAccessMode.None;

    /// <summary>
    /// Initial layout of <see cref="Texture"/>.
    /// </summary>
    public ResourceStates InitialLayout { get; set; } = ResourceStates.ShaderResource;

    /// <summary>
    /// Gets or sets the label of <see cref="Texture"/>.
    /// </summary>
    public string? Label { get; init; }
}
