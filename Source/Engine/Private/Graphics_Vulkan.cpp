// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_RHI_VULKAN)
#include "Graphics/Graphics.h"
#include "Graphics/Texture.h"
#include "Platform/Window.h"
#include "PlatformInclude.h"
#include "volk.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "spirv_reflect.h"

namespace alimer
{
    namespace
    {
    }

    static struct
    {
    } vulkan;

    class Vulkan_Texture final : public RefCounter<Texture>
    {
    public:
        Vulkan_Texture(u32 width, u32 height)
        {
            
        }

        ~Vulkan_Texture() override
        {

        }
    };

    class Vulkan_Graphics final : public Graphics
    {
    private:

    public:
        Vulkan_Graphics();
        ~Vulkan_Graphics() override;

        bool Initialize(_In_ Window* window) override;
        bool BeginFrame() override;
        void EndFrame() override;
        RefCountPtr<Texture> CreateTexture(u32 width, u32 height) override;
    };

    Vulkan_Graphics::Vulkan_Graphics()
    {
    }

    Vulkan_Graphics::~Vulkan_Graphics()
    {
        
    }

    bool Vulkan_Graphics::Initialize(_In_ Window* window)
    {
        return true;
    }

    bool Vulkan_Graphics::BeginFrame()
    {
        return true;
    }

    void Vulkan_Graphics::EndFrame()
    {
    }

    RefCountPtr<Texture> Vulkan_Graphics::CreateTexture(u32 width, u32 height)
    {
        auto result = new Vulkan_Texture(width, height);

        //if (result->handle)
            return TextureRef::Create(result);

        //delete result;
        //return nullptr;
    }
}

namespace alimer
{
    bool InitializeVulkan(Window* window)
    {
        gGraphics().Start<Vulkan_Graphics>();
        return gGraphics().Initialize(window);
    }
}

#endif /* defined(ALIMER_RHI_VULKAN) */
