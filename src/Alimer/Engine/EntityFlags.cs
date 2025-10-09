// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Engine;

/// <summary>
/// Defines <see cref="Entity"/> flags.
/// </summary>
[Flags]
public enum EntityFlags
{
    /// <summary>
    /// None
    /// </summary>
    None = 0,

    /// <summary>
    /// Don't save this entity to disk, or when duplicating.
    /// </summary>
    NotSaved = (1 << 0),

    /// <summary>
    /// Hide entity in hierarchy.
    /// </summary>
    HideInHierarchy = (1 << 1),

    /// <summary>
    /// Hide entity in inspector.
    /// </summary>
    HideInInspector = (1 << 2),

    /// <summary>
    /// Hide entity in hierarchy and inspector.
    /// </summary>
    Hidden = HideInHierarchy | HideInInspector,
}
