﻿// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;

namespace Alimer.Graphics.D3D12;

internal interface ID3D12GpuResource
{
    unsafe ID3D12Resource* Handle { get; }
    ResourceStates State { get; set; }

    ResourceStates TransitioningState { get; set; }
}
