// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/RefCount.h"
#include "Core/Module.h"
#include "Graphics/CommandList.h"

namespace alimer
{
    class Window;
    class Texture;
    struct TextureDesc;
    struct TextureData;

    enum class ValidationMode : uint32_t
    {
        /// No validation is enabled.
        Disabled,
        /// Print warnings and errors
        Enabled,
        /// Print all warnings, errors and info messages
        Verbose,
        /// Enable GPU-based validation
        GPU
    };

    struct PresentationParameters
    {
        ValidationMode validationMode = ValidationMode::Disabled;
        uint32_t maxFramesInFlight = 2;

        uint32_t backBufferWidth = 0;
        uint32_t backBufferHeight = 0;
        uint32_t backBufferCount = 3;
        Format depthStencilFormat = Format::Depth32Float;
        bool vsyncEnabled = false;
        bool isFullScreen = false;
    };

    class ALIMER_API Graphics : public Module<Graphics>
    {
        friend class Texture;

    public:
        virtual bool Initialize(_In_ Window* window, const PresentationParameters& presentationParameters) = 0;
        virtual void WaitIdle() = 0;
        virtual CommandList* BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Resize(u32 newWidth, u32 newHeight) = 0;

        /// Return backbuffer width.
        [[nodiscard]] uint32_t GetBackBufferWidth() const { return backBufferWidth; }
        /// Return backbuffer height.
        [[nodiscard]] uint32_t GetBackBufferHeight() const { return backBufferHeight; }

    private:
        [[nodiscard]] virtual RefCountPtr<Texture> CreateTexture(const TextureDesc& desc, const TextureData* initialData = nullptr) = 0;

    protected:
        uint32_t backBufferWidth = 0;
        uint32_t backBufferHeight = 0;
        bool vsyncEnabled = false;
    };

    ALIMER_API bool GraphicsInitialize(_In_ Window* window, const PresentationParameters& presentationParameters);

    /** Provides easier access to graphics module. */
    ALIMER_API Graphics& gGraphics();
}

