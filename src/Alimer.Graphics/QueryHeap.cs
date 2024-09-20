// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Type of query contained in a <see cref="QueryHeap"/>.
/// </summary>
public enum QueryType
{
    /// <summary>
    /// Used for occlusion query heap or occlusion queries
    /// </summary>
    Occlusion,
    /// <summary>
    /// Can be used in the same heap as occlusion
    /// </summary>
    BinaryOcclusion,
    /// <summary>
    /// Create a heap to contain timestamp queries
    /// </summary>
    Timestamp,
    /// <summary>
    /// Create a heap to contain a structure of <see cref="QueryDataPipelineStatistics"/>
    /// </summary>
    PipelineStatistics,
}


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

/// <summary>
/// Structure that describes the <see cref="QueryHeap"/>.
/// </summary>
public readonly record struct QueryHeapDescriptor
{
    public QueryHeapDescriptor(QueryType type = QueryType.Timestamp, int count = 1, string? label = default)
    {
        Guard.IsTrue(count > 0 && count < Constants.QuerySetMaxQueries);

        Type = type;
        Count = count;
        Label = label;
    }

    /// <summary>
    /// Kind of query that this query set should contain.
    /// </summary>
    public QueryType Type { get; init; }

    /// <summary>
    /// Total count of queries the set contains. Must not be zero. Must not be greater than <see cref="Constants.QuerySetMaxQueries"/>.
    /// </summary>
    public int Count { get; init; }

    /// <summary>
    /// 
    /// </summary>
    public QueryPipelineStatisticFlags PipelineStatistics { get; init; } = QueryPipelineStatisticFlags.None;

    /// <summary>
    /// Gets or sets the debug label for the query set.
    /// </summary>
    public string? Label { get; init; }
}

/// <summary>
/// Defines a query heap.
/// </summary>
public abstract class QueryHeap : GraphicsObject
{
    protected QueryHeap(in QueryHeapDescriptor descriptor)
        : base(descriptor.Label)
    {
        Type = descriptor.Type;
        Count = descriptor.Count;
    }

    public QueryType Type { get; }
    public int Count { get; }
}
