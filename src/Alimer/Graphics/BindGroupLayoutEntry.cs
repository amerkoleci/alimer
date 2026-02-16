// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public readonly struct BufferBindingLayout
{
    public readonly BufferBindingType Type = BufferBindingType.Constant;
    public readonly bool HasDynamicOffset = false;

    public BufferBindingLayout()
    {
    }

    public BufferBindingLayout(BufferBindingType type, bool hasDynamicOffset = false)
    {
        Type = type;
        HasDynamicOffset = hasDynamicOffset;
    }
}

public readonly struct SamplerBindingLayout
{
    public readonly SamplerBindingType Type = SamplerBindingType.Filtering;

    public SamplerBindingLayout()
    {
    }

    public SamplerBindingLayout(SamplerBindingType type = SamplerBindingType.Filtering)
    {
        Type = type;
    }
}

public readonly struct TextureBindingLayout
{
    public readonly TextureSampleType SampleType = TextureSampleType.Float;
    public readonly TextureViewDimension ViewDimension = TextureViewDimension.View2D;
    public readonly bool Multisampled = false;

    public TextureBindingLayout()
    {
    }

    public TextureBindingLayout(TextureSampleType sampleType = TextureSampleType.Float, bool multisampled = false)
    {
        SampleType = sampleType;
        Multisampled = multisampled;
    }
}

public readonly struct StorageTextureBindingLayout
{
    public readonly StorageTextureAccess Access = StorageTextureAccess.WriteOnly;
    public readonly PixelFormat Format = PixelFormat.Undefined;
    public readonly TextureViewDimension ViewDimension = TextureViewDimension.View2D;

    public StorageTextureBindingLayout()
    {
    }

    public StorageTextureBindingLayout(StorageTextureAccess access, PixelFormat format = PixelFormat.Undefined)
    {
        Access = access;
        Format = format;
    }
}

public readonly struct AccelerationStructureBindingLayout
{
    public readonly PixelFormat Format = PixelFormat.Undefined;

    public AccelerationStructureBindingLayout()
    {
    }

    public AccelerationStructureBindingLayout(PixelFormat format)
    {
        Format = format;
    }
}

/// <summary>
/// Single entry for <see cref="BindGroupLayout"/>.
/// </summary>
public readonly struct BindGroupLayoutEntry
{
    /// <summary>
    /// Register index to bind to (supplied in shader).
    /// </summary>
    public readonly uint Binding;

    /// <summary>
    /// The shader stage the resources will be accessible to.
    /// </summary>
    public readonly ShaderStages Visibility;

    /// <summary>
    /// The number of descriptors contained in the binding, accessed in a shader as an array.
    /// </summary>
    public readonly uint Count = 1u;

    /// <summary>
    /// Gets the buffer binding.
    /// </summary>
    public readonly BufferBindingLayout Buffer;

    public readonly SamplerBindingLayout Sampler;

    public readonly SamplerDescriptor? StaticSampler;

    public readonly TextureBindingLayout Texture;
    public readonly StorageTextureBindingLayout StorageTexture;
    public readonly AccelerationStructureBindingLayout AccelerationStructure;

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
            else if (AccelerationStructure.Format != PixelFormat.Undefined)
            {
                return BindingInfoType.AccelerationStructure;
            }
            else
            {
                return BindingInfoType.Undefined;
            }
        }
    }
}
