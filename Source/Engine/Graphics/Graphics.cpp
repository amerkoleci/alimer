// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Graphics.h"
#include "Graphics/Texture.h"
#include "Core/Log.h"
#include "Window.h"

namespace Alimer::rhi
{
#if defined(ALIMER_RHI_D3D12)
    extern bool InitializeD3D12Backend(Window* window, const PresentationParameters& presentationParameters);
#endif

#if defined(ALIMER_RHI_VULKAN)
    extern bool InitializeVulkanBackend(Window* window, const PresentationParameters& presentationParameters);
#endif
}

using namespace Alimer::rhi;

namespace Alimer
{
    bool Graphics::Initialize(_In_ Window* window, const PresentationParameters& presentationParameters)
    {
#if defined(ALIMER_RHI_D3D12)
        return InitializeD3D12Backend(window, presentationParameters);
#endif

#if defined(ALIMER_RHI_VULKAN)
        //return InitializeVulkanBackend(window, presentationParameters);
#endif

        return false;
    }

    PipelineHandle Graphics::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        RenderPipelineDesc descDef = desc;

        uint32_t autoOffsets[kMaxVertexBufferBindings] = {};

        bool useAutoOffset = true;
        for (uint32_t index = 0; index < kMaxVertexAttributes; index++)
        {
            // To use computed offsets, all attribute offsets must be 0.
            if (desc.vertexLayout.attributes[index].offset != 0) {
                useAutoOffset = false;
                break;
            }
        }

        for (uint32_t index = 0; index < kMaxVertexAttributes; index++)
        {
            VertexAttribute* attribute = &descDef.vertexLayout.attributes[index];
            if (attribute->format == VertexFormat::Undefined) {
                continue;
            }

            ALIMER_ASSERT(attribute->bufferIndex < kMaxVertexBufferBindings);
            if (useAutoOffset) {
                attribute->offset = autoOffsets[attribute->bufferIndex];
            }
            autoOffsets[attribute->bufferIndex] += GetVertexFormatSize(attribute->format);
        }

        // Compute vertex strides if needed.
        for (uint32_t index = 0; index < kMaxVertexBufferBindings; index++)
        {
            VertexBufferLayout* layout = &descDef.vertexLayout.buffers[index];
            if (layout->stride == 0) {
                layout->stride = autoOffsets[index];
            }
        }

        return CreateRenderPipelineCore(descDef);
    }

    Graphics& gGraphics()
    {
        return Graphics::Instance();
    }
}
