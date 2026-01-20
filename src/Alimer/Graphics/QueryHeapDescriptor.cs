// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="QueryHeap"/>.
/// </summary>
public  record struct QueryHeapDescriptor
{
    /// <summary>
    /// Kind of query that this query set should contain.
    /// </summary>
    public QueryType Type;

    /// <summary>
    /// Total count of queries the set contains. Must not be zero. Must not be greater than <see cref="Constants.QuerySetMaxQueries"/>.
    /// </summary>
    public required uint Count;

    /// <summary>
    /// Gets or sets the debug label for the query set.
    /// </summary>
    public string? Label;

    [SetsRequiredMembers]
    public QueryHeapDescriptor(QueryType type = QueryType.Timestamp, uint count = 1, string? label = default)
    {
        Type = type;
        Count = count;
        Label = label;
    }

}
