// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsDefs.h"
#define D3D11_NO_HELPERS
#include <d3d11_1.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace Alimer
{
    // Type alias for ComPtr template
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
}

