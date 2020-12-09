// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Alimer.Graphics
{
    public enum BackendType
    {
        /// <summary>
        /// Null renderer.
        /// </summary>
        Null,
        /// <summary>
        /// Vulkan backend.
        /// </summary>
        Vulkan,
        /// <summary>
        /// Direct3D 12 backend.
        /// </summary>
        Direct3D12,
        /// <summary>
        /// Metal backend.
        /// </summary>
        Metal,
        /// <summary>
        /// Default best platform supported backend.
        /// </summary>
        Count
    }
}
