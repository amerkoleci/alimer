// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.D3D12;

internal static class Extensions
{
    extension(GpuBuffer buffer)
    {
        internal D3D12Buffer ToD3D12()
        {
            return (D3D12Buffer)buffer;
        }
    }

    extension(Texture texture)
    {
        internal D3D12Texture ToD3D12()
        {
            return (D3D12Texture)texture;
        }
    }

    extension(TextureView textureView)
    {
        internal D3D12TextureView ToD3D12()
        {
            return (D3D12TextureView)textureView;
        }
    }

    extension(QueryHeap queryHeap)
    {
        internal D3D12QueryHeap ToD3D12()
        {
            return (D3D12QueryHeap)queryHeap;
        }
    }
}
