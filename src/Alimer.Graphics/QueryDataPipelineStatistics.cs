// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// GPU pipeline statistics obtain by using a `<see cref="QueryType.PipelineStatistics"/>` query.
/// </summary>
public readonly record struct QueryDataPipelineStatistics
{
    public ulong InputAssemblyVertices { get; init; }
    public ulong InputAssemblyPrimitives { get; init; }
    public ulong VertexShaderInvocations { get; init; }
    public ulong GeometryShaderInvocations { get; init; }
    public ulong GeometryShaderPrimitives { get; init; }
    public ulong ClippingInvocations { get; init; }
    public ulong ClippingPrimitives { get; init; }
    public ulong PixelShaderInvocations { get; init; }
    public ulong HullShaderInvocations { get; init; }
    public ulong DomainShaderInvocations { get; init; }
    public ulong ComputeShaderInvocations { get; init; }
    public ulong AmplificationShaderInvocations { get; init; }
    public ulong MeshShaderInvocations { get; init; }
}
