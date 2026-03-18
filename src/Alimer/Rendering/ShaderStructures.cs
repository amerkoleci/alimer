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

public struct LightData : IShaderConstantBuffer
{
    public float3 position;
    private float _padding0;
    public float3 direction;
    private float _padding1;
    public float3 color;
    public float intensity;
    public float range;
    public float innerConeCos;
    public float outerConeCos;
    public ShaderLightType type;
}

public struct InstanceData : IShaderConstantBuffer
{
    public Matrix4x4 worldMatrix;
    //public Color color;
    //public uint materialIndex;
}

// Updated once per material
public struct PBRMaterialUniforms : IShaderConstantBuffer
{
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
    public uint baseColorTextureIndex;
    public uint samplerIndex;
}
