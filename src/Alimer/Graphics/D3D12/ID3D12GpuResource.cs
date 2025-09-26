// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;

namespace Alimer.Graphics.D3D12;

internal unsafe interface ID3D12GpuResource
{
    ID3D12Resource* Handle { get; }
}
