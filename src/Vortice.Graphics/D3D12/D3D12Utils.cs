// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using Microsoft.Toolkit.Diagnostics;
using Vortice.Direct3D12;
using Vortice.DXGI;

namespace Vortice.Graphics.D3D12
{
    internal static unsafe class D3D12Utils
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static ResourceDimension ToD3D12(this TextureDimension dimension)
        {
            switch (dimension)
            {
                case TextureDimension.Texture1D:
                    return ResourceDimension.Texture1D;
                case TextureDimension.Texture2D:
                    return ResourceDimension.Texture2D;
                case TextureDimension.Texture3D:
                    return ResourceDimension.Texture3D;

                default:
                    return ThrowHelper.ThrowArgumentException<ResourceDimension>("Invalid texture dimension");
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static CommandListType ToD3D12(this CommandQueueType type)
        {
            switch (type)
            {
                case CommandQueueType.Compute:
                    return CommandListType.Compute;
                case CommandQueueType.Copy:
                    return CommandListType.Copy;

                default:
                    return CommandListType.Direct;
            }
        }
    }
}
