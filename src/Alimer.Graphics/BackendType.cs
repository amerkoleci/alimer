// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;

namespace Alimer.Graphics
{
    public enum BackendType
    {
        Default,
        Null,
        Direct3D11,
        Direct3D12,
        Metal,
        Vulkan,
        OpenGL,
    }
}