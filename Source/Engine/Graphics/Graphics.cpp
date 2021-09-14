// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "AlimerConfig.h"
#include "Graphics/Graphics.h"
#include "Graphics/Texture.h"
#include "Core/Log.h"
#include "Window.h"

#if defined(ALIMER_GRAPHICS_D3D11)
#include "Graphics/D3D11/D3D11Graphics.h"
#endif

#if defined(ALIMER_GRAPHICS_GL)
#include "Graphics/GL/GLGraphics.h"
#endif

namespace Alimer
{
    Graphics::Graphics(Window& window)
        : window{ window }
    {
    }

    std::set<GraphicsAPI> Graphics::GetAvailableBackends()
    {
        static std::set<GraphicsAPI> availableDrivers;

        if (availableDrivers.empty())
        {
            availableDrivers.insert(GraphicsAPI::Null);

#if defined(ALIMER_GRAPHICS_D3D11)
            availableDrivers.insert(GraphicsAPI::D3D11);
#endif

#if defined(ALIMER_GRAPHICS_GL)
            availableDrivers.insert(GraphicsAPI::OpenGL);
#endif

#if defined(ALIMER_GRAPHICS_METAL)
            if (MetalGraphics::IsAvailable())
                availableDrivers.insert(GraphicsAPI::Metal);
#endif
        }

        return availableDrivers;
    }

    GraphicsAPI Graphics::GetBestPlatformAPI()
    {
#if defined(ALIMER_GRAPHICS_D3D11)
        return GraphicsAPI::D3D11;
#endif

        return GraphicsAPI::OpenGL;
    }

    bool Graphics::Initialize(GraphicsAPI api, Window& window, const GraphicsCreateInfo& createInfo)
    {
#if defined(ALIMER_GRAPHICS_D3D11)
        if (api == GraphicsAPI::D3D11)
        {
            gGraphics().Start(new D3D11GraphicsDevice(window, createInfo));
        }

#endif
#if defined(ALIMER_GRAPHICS_GL)
        if (!gGraphics().IsInitialized())
        {
            gGraphics().Start(new GLGraphicsDevice(window, createInfo));
        }
#endif

        return gGraphics().IsInitialized();
    }


    Graphics& gGraphics()
    {
        return Graphics::Instance();
    }
}
