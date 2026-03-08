// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Rendering;

namespace Alimer.Engine;

/// <summary>
/// A component that contains a mesh and materials for the visual appearance of an entity.
/// </summary>
[Meta]
[DefaultEntitySystem(typeof(LightSystem))]
public sealed partial class LightComponent : Component
{
    public LightComponent()
    {
    }

    public LightType LightType { get; set; } = LightType.Point;
    public Color Color { get; set; } = Colors.White;
    public float Intensity { get; set; } = 1.0f;
}
