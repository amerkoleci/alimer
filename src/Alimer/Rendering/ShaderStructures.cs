// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Graphics;
using float2 = System.Numerics.Vector2;
using float3 = System.Numerics.Vector3;
using float4 = System.Numerics.Vector4;
using float4x4 = System.Numerics.Matrix4x4;

#pragma warning disable CS0169 // warning CS0169: The field '_padding0' is never used

namespace Alimer.Rendering;

// TODO: Source generator for defined types
// Must match shader layout (ShaderTypes.h)

// Updated once per frame
public struct FrameConstants : IShaderConstantBuffer
{
    public float ElapsedTime;
    public float TotalTime;
    private float2 _padding0; // Padding to align to 16 bytes
}

// Updated once per camera/view
public struct PerViewData : IShaderConstantBuffer
{
    public float4x4 viewMatrix;
    public float4x4 projectionMatrix;
    public float4x4 viewProjectionMatrix;
    public float4x4 inverseViewMatrix;
    public float4x4 inverseProjectionMatrix;
    public float3 cameraPosition;
    private float _padding0; // Padding to align to 16 bytes
    public float3 ambientLight;
    public uint activeLightCount;
}

// Updated once per camera/view
public enum ShaderLightType
{
    Invalid,
    Directional,
    Point,
    Spot,
}

public struct GpuLight : IShaderConstantBuffer
{
    public float3 Position;         // World-space position (unused for directional)
    public ShaderLightType Type;    // 0=Directional, 1=Point, 2=Spot
    public float3 Color;            // Linear RGB × intensity
    public float Range;             // Effective falloff range
    public float3 Direction;
    private float _padding0;
    public float InnerConeCos;
    public float OuterConeCos;
    private Vector2 _padding1;
}

public struct GPUInstance : IShaderConstantBuffer
{
    public Matrix4x4 worldMatrix;
    public int MaterialIndex;    // Index into the material buffer
    public int VertexBufferIndex;// Bindless handle to the model's vertex buffer
    public int BaseVertex;       // Base vertex offset within the vertex buffer
    private int _pad0;
}

// Updated once per material
public struct GPUMaterialPBR : IShaderConstantBuffer
{
    public int BaseIndex;
    public int NormalIndex;
    public int MetallicRoughnessIndex;
    public int EmissiveIndex;
    public int OcclusionIndex;
    public int SamplerIndex;
    private Vector2 _padding;
    public Color baseColorFactor;
    public Vector3 emissiveFactor;
    public float normalScale; 
    public float2 metallicRoughnessFactor;
    public float occlusionStrength;
    public float alphaCutoff;
    public int baseColorUVSet;
    public int normalUVSet;
    public int metallicRoughnessUVSet;
    public int emissiveUVSet;
}

public struct PBRPushConstants
{
    public int InstanceBufferIndex;
    public int MaterialBufferIndex;
    public int LightBufferIndex;
    private int _pad;
}
