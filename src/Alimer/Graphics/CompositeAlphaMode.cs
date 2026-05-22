// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum CompositeAlphaMode 
{
    /// <summary>
    /// Lets the backend implementation choose the best mode (supported, and with the best performance) between RHICompositeAlphaMode::Opaque or RHICompositeAlphaMode::Inherit.
    /// </summary>
    Auto = 0,
    /// <summary>
    /// The alpha component of the image is ignored and teated as if it is always 1.0.
    /// </summary>
    Opaque,
    /// <summary>
    /// The alpha component is respected and non-alpha components are assumed to be already multiplied with the alpha component. For example, (0.5, 0, 0, 0.5) is semi-transparent bright red.
    /// </summary>
    Premultiplied,
    /// <summary>
    /// The alpha component is respected and non-alpha components are assumed to NOT be already multiplied with the alpha component. For example, (1.0, 0, 0, 0.5) is semi-transparent bright red.
    /// </summary>
    Unpremultiplied,
    /// <summary>
    /// The handling of the alpha component is unknown to WebGPU and should be handled by the application using system-specific APIs. This mode may be unavailable (for example on Wasm).
    /// </summary>
    Inherit,
}
