// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Vortice.Graphics
{
    public abstract class SwapChain : GraphicsResource
    {
        protected SwapChain(GraphicsDevice device, in SwapChainDescriptor descriptor)
            : base(device)
        {
        }
    }
}
