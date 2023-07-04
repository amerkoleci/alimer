// Copyright � Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Defines a query heap.
/// </summary>
public abstract class QueryHeap : GraphicsObject
{
    protected QueryHeap(in QueryHeapDescription description)
        : base(description.Label)
    {
        Type = description.Type;
        Count = description.Count;
    }

    public QueryType Type { get; }
    public int Count { get; }
}
