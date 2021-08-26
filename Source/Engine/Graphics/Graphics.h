// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Types.h"
#include "Core/RefCount.h"
#include "Core/Module.h"

namespace alimer
{
    class Window;
    class Texture;

    class ALIMER_API Graphics : public Module<Graphics>
    {
        friend class Texture;

    public:
        virtual bool Initialize(_In_ Window* window) = 0;
        virtual bool BeginFrame() = 0;
        virtual void EndFrame() = 0;

    private:
        [[nodiscard]] virtual RefCountPtr<Texture> CreateTexture(u32 width, u32 height) = 0;
    };

    ALIMER_API bool GraphicsInitialize(_In_ Window* window);

    /** Provides easier access to graphics module. */
    ALIMER_API Graphics& gGraphics();
}

