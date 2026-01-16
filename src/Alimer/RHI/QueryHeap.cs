// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.RHI;

/// <summary>
/// Defines a query heap.
/// </summary>
public abstract class QueryHeap : RHIObject
{
    protected QueryHeap(in QueryHeapDescriptor descriptor)
        : base(descriptor.Label)
    {
        Type = descriptor.Type;
        Count = descriptor.Count;
    }

    public QueryType Type { get; }
    public uint Count { get; }

    /// <summary>
    /// Gets the size, in bytes of the query result.
    /// </summary>
    public abstract uint QueryResultSize { get; }
}
