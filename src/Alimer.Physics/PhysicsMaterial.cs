// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Physics;

/// <summary>
/// How two materials combine at a contact; the pair's effective mode is the max of both bodies modes(Max > Min > Multiply > Average), so a "sticky" material wins.
/// </summary>
public enum PhysicsMaterialCombineMode 
{
    Average,
    Min,
    Multiply,
    Max,
}

public class PhysicsMaterial
{
    /// <summary>
    /// Surface friction coefficient. 0 = ice, 1 = rubber on rubber.
    /// </summary>
    public float Friction { get; set; } = 0.4f;

    /// <summary>
    /// Bounciness: 0 = inelastic, 1 = perfectly elastic.
    /// </summary>
    public float Restitution { get; set; } = 0.3f;

    /// <summary>
    /// Volumetric density in kg/m^3. Used when the rigid body computes mass from shape volume.
    /// </summary>
    public float Density { get; set; } = 1000.0f;

    /// <summary>
    /// Rule for combining this surface's friction with another at contact.
    /// </summary>
    public PhysicsMaterialCombineMode FrictionCombine { get; set; } = PhysicsMaterialCombineMode.Average;

    /// <summary>
    /// Rule for combining this surface's restitution with another at contact.
    /// </summary>
    public PhysicsMaterialCombineMode RestitutionCombine { get; set; } = PhysicsMaterialCombineMode.Max;
}

