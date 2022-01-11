// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Microsoft.Toolkit.Diagnostics;
using Vortice.Direct3D12;

namespace Vortice.Graphics;

internal static class D3D12Utils
{
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static ResourceDimension ToD3D12(this TextureDimension dimension)
    {
        return dimension switch
        {
            TextureDimension.Texture1D => ResourceDimension.Texture1D,
            TextureDimension.Texture2D => ResourceDimension.Texture2D,
            TextureDimension.Texture3D => ResourceDimension.Texture3D,
            TextureDimension.TextureCube => ResourceDimension.Texture2D,
            _ => ThrowHelper.ThrowArgumentException<ResourceDimension>("Invalid texture dimension"),
        };
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static CommandListType ToD3D12(this CommandQueueType type)
    {
        return type switch
        {
            CommandQueueType.Compute => CommandListType.Compute,
            CommandQueueType.Copy => CommandListType.Copy,
            _ => CommandListType.Direct,
        };
    }
}
