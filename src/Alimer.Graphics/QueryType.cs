// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

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
