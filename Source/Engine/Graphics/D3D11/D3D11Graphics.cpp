// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D11Graphics.h"
#include "Core/Log.h"
#include "Window.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace Alimer
{
    D3D11Graphics::D3D11Graphics(Window& window, const PresentationParameters& presentationParameters)
        : Graphics(window)
    {
    }

    D3D11Graphics::~D3D11Graphics()
    {

    }
}
