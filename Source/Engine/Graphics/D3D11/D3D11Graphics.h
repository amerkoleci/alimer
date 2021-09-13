// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Graphics.h"
#include "D3D11Backend.h"

namespace Alimer
{
    class D3D11Graphics final : public Graphics
    {
    public:
        D3D11Graphics(Window& window, const PresentationParameters& presentationParameters);
        ~D3D11Graphics() override;

    private:
    };
}

