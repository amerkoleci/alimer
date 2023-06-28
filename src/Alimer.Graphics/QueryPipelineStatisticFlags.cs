// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

[Flags]
public enum QueryPipelineStatisticFlags
{
    None = 0,
    InputAssemblyVertices = (1 << 0),
    InputAssemblyPrimitives = (1 << 1),
    VertexShaderInvocations = (1 << 2),
    GeometryShaderInvocations = (1 << 3),
    GeometryShaderPrimitives = (1 << 4),
    ClippingInvocations = (1 << 5),
    ClippingPrimitives = (1 << 6),
    PixelShaderInvocations = (1 << 7),
    HullShaderInvocations = (1 << 8),
    DomainShaderInvocations = (1 << 9),
    ComputeShaderInvocations = (1 << 10),
    AmplificationShaderInvocations = (1 << 11),
    MeshShaderInvocations = (1 << 12),
}
