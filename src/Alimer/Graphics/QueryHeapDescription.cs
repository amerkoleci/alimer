// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="QueryHeap"/>.
/// </summary>
public readonly record struct QueryHeapDescription
{
    public QueryHeapDescription(QueryType type = QueryType.Timestamp, int count = 1, string? label = default)
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
