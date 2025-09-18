// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public struct GraphicsManagerOptions
{
    public GraphicsBackendType PreferredBackend { get; set; } = GraphicsBackendType.Default;
    public GraphicsValidationMode ValidationMode { get; set; } = GraphicsValidationMode.Disabled;
    public string? Label { get; set; }

    public GraphicsManagerOptions()
    {

    }
}
