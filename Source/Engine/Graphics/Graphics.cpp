// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Texture.h"
#include "Graphics/Buffer.h"
#include "Graphics/Pipeline.h"
#include "Graphics/SwapChain.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

#if defined(ALIMER_GRAPHICS_D3D12) && defined(TODO)
#   include "Graphics/D3D12/D3D12Graphics.h"
#endif

#if defined(ALIMER_GRAPHICS_VULKAN)
#include "Graphics/Vulkan/VulkanGraphics.h"
#endif

namespace Alimer
{
    void Graphics::Destroy()
    {
        // Stock Samplers
        {
            //pointClampSampler.Reset();
            //pointWrapSampler.Reset();
            //linearClampSampler.Reset();
            //linearWrapSampler.Reset();
        }

        // Destroy pending resources that still exist.
        {
            std::lock_guard<std::mutex> lock(objectsMutex);

            for (GPUObject* resource : objects)
            {
                resource->Destroy();
            }

            objects.clear();
        }
    }

    bool Graphics::Initialize(GraphicsAPI api, ValidationMode validationMode)
    {
#if defined(ALIMER_GRAPHICS_D3D12) && defined(TODO)
        if (api == GraphicsAPI::D3D12)
        {
            gGraphics().Start(new D3D12Graphics(validationMode));
        }
#endif
#if defined(ALIMER_GRAPHICS_VULKAN)
        if (!gGraphics().IsInitialized())
        {
            gGraphics().Start(new VulkanGraphics(validationMode));
        }
#endif

        return gGraphics().IsInitialized();
    }

	void Graphics::AddGPUObject(GPUObject* resource)
	{
        std::lock_guard<std::mutex> lock(objectsMutex);
        auto it = std::find(objects.begin(), objects.end(), resource);
        if (it == objects.end())
        {
            objects.push_back(resource);
        }
        else
        {
            LOGD("GPUObject already tracked");
        }
	}

	void Graphics::RemoveGPUObject(GPUObject* resource)
	{
		std::lock_guard<std::mutex> lock(objectsMutex);
		auto it = std::remove(objects.begin(), objects.end(), resource);
		if (it != objects.end())
		{
            objects.erase(it);
		}
	}

    TextureRef Graphics::CreateTexture(const TextureCreateInfo& info, const void* initialData)
	{
		ALIMER_ASSERT(info.width >= 1);

		return CreateTextureCore(info, initialData);
	}

	Graphics& gGraphics()
	{
		return Graphics::Instance();
	}
}
