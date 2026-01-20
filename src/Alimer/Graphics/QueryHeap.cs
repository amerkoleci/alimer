// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Defines a query heap.
/// </summary>
public abstract class QueryHeap(in QueryHeapDescriptor descriptor) : GraphicsObject(descriptor.Label)
{
    public QueryType Type { get; } = descriptor.Type;
    public uint Count { get; } = descriptor.Count;

    /// <summary>
    /// Gets the size, in bytes of the query result.
    /// </summary>
    public abstract uint QueryResultSize { get; }
}
