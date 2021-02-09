// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Vortice.Graphics
{
    public enum BackendType
    {
        /// <summary>
        /// Default best platform supported backend.
        /// </summary>
        Default,
        /// <summary>
        /// Direct3D 12 backend.
        /// </summary>
        Direct3D12,
        /// <summary>
        /// Vulkan backend.
        /// </summary>
        Vulkan,
        /// <summary>
        /// Null renderer.
        /// </summary>
        Null,
    }
}
