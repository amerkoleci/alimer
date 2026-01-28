// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Graphics;

namespace Alimer.Rendering;

// TODO: Source generator for defined types
// Must match shader layout (ShaderTypes.h)

// Updated once per frame
public struct FrameConstants : IShaderConstantBuffer
{
    public float ElapsedTime { get; set; }
    public float TotalTime { get; set; }
}

// Updated once per camera/view
public struct PerViewData : IShaderConstantBuffer
{
    public Matrix4x4 viewMatrix;
    public Matrix4x4 projectionMatrix;
    public Matrix4x4 viewProjectionMatrix;
    public Matrix4x4 inverseViewMatrix;
    public Matrix4x4 inverseProjectionMatrix;
    public Vector3 cameraPosition;
}

// Updated once per material
public struct PBRMaterialData : IShaderConstantBuffer
{
    public Color baseColorFactor;
    public Vector4 emissiveFactor;
    public float metallicFactor;
    public float roughnessFactor;
    public float normalScale;
    public float occlusionStrength;
}
