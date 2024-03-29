﻿// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Single entry for <see cref="BindGroupLayout"/>.
/// </summary>
public readonly record struct BindGroupLayoutEntry
{
    public BindGroupLayoutEntry(BufferBindingLayout buffer, uint binding, ShaderStages visibility = ShaderStages.All)
    {
        Binding = binding;
        Visibility = visibility;
        Buffer = buffer;
    }

    public BindGroupLayoutEntry(SamplerBindingLayout sampler, uint binding, ShaderStages visibility = ShaderStages.All)
    {
        Binding = binding;
        Visibility = visibility;
        Sampler = sampler;
    }

    public BindGroupLayoutEntry(SamplerDescriptor staticSampler, uint binding, ShaderStages visibility = ShaderStages.All)
    {
        Binding = binding;
        Visibility = visibility;
        StaticSampler = staticSampler;
    }

    public BindGroupLayoutEntry(TextureBindingLayout texture, uint binding, ShaderStages visibility = ShaderStages.All)
    {
        Binding = binding;
        Visibility = visibility;
        Texture = texture;
    }

    public BindGroupLayoutEntry(StorageTextureBindingLayout storageTexture, uint binding, ShaderStages visibility = ShaderStages.All)
    {
        Binding = binding;
        Visibility = visibility;
        StorageTexture = storageTexture;
    }

    /// <summary>
    /// Register index to bind to (supplied in shader).
    /// </summary>
    public uint Binding { get; init; }

    /// <summary>
    /// The shader stage the resources will be accessible to.
    /// </summary>
    public ShaderStages Visibility { get; init; }

    /// <summary>
    /// Gets the buffer binding.
    /// </summary>
    public BufferBindingLayout Buffer { get; init; }

    public SamplerBindingLayout Sampler { get; init; }

    public SamplerDescriptor? StaticSampler { get; init; }

    public TextureBindingLayout Texture { get; init; }
    public StorageTextureBindingLayout StorageTexture { get; init; }

    public BindingInfoType BindingType
    {
        get
        {
            if (Buffer.Type != BufferBindingType.Undefined)
            {
                return BindingInfoType.Buffer;
            }
            else if (Sampler.Type != SamplerBindingType.Undefined || StaticSampler.HasValue)
            {
                return BindingInfoType.Sampler;
            }
            else if (Texture.SampleType != TextureSampleType.Undefined)
            {
                return BindingInfoType.Texture;
            }
            else if (StorageTexture.Access != StorageTextureAccess.Undefined)
            {
                return BindingInfoType.StorageTexture;
            }
            else
            {
                return BindingInfoType.Undefined;
            }
        }
    }
}

public readonly struct BufferBindingLayout
{
    public readonly BufferBindingType Type;
    public readonly bool HasDynamicOffset;
    public readonly ulong MinBindingSize;

    public BufferBindingLayout()
    {
        Type = BufferBindingType.Constant;
        HasDynamicOffset = false;
        MinBindingSize = 0;
    }

    public BufferBindingLayout(BufferBindingType type, bool hasDynamicOffset = false, ulong minBindingSize = 0)
    {
        Type = type;
        HasDynamicOffset = hasDynamicOffset;
        MinBindingSize = minBindingSize;
    }
}

public readonly struct SamplerBindingLayout
{
    public readonly SamplerBindingType Type;

    public SamplerBindingLayout()
    {
        Type = SamplerBindingType.Filtering;
    }

    public SamplerBindingLayout(SamplerBindingType type)
    {
        Type = type;
    }
}

public readonly struct TextureBindingLayout
{
    public readonly TextureSampleType SampleType;
    //public TextureViewDimension ViewDimension;
    public readonly bool Multisampled;

    public TextureBindingLayout()
    {
        SampleType = TextureSampleType.Float;
        Multisampled = false;
    }

    public TextureBindingLayout(TextureSampleType sampleType, bool multisampled = false)
    {
        SampleType = sampleType;
        Multisampled = multisampled;
    }
}

public readonly struct StorageTextureBindingLayout
{
    public readonly StorageTextureAccess Access;
    public readonly PixelFormat Format;
    //public TextureViewDimension viewDimension;

    public StorageTextureBindingLayout()
    {
        Access = StorageTextureAccess.WriteOnly;
        Format = PixelFormat.Undefined;
    }

    public StorageTextureBindingLayout(StorageTextureAccess access, PixelFormat format = PixelFormat.Undefined)
    {
        Access = access;
        Format = format;
    }
}
