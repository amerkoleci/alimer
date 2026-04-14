// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Object.h"
#include "Alimer/Core/Signal.h"
#include "Alimer/Platform/Types.h"
#include "Alimer/Math/Vector2.h"
#include "Alimer/RHI/RHI.h"

namespace Alimer
{
    struct WindowImpl;

    /// Class that defines an OS window.
    class ALIMER_API Window final
    {
        friend class Application;

    public:
        /// Occurs when the window is resized.
        Signal<> Resized;

        /// Constructor.
        Window(const std::string& title, uint32_t width, uint32_t height, WindowFlags flags = WindowFlags::Resizable);

        /// Destructor.
        ~Window();

        // Non-copyable and non-movable
        ALIMER_DISABLE_COPY_MOVE(Window);

        [[nodiscard]] uint32_t GetId() const { return _id; }
        [[nodiscard]] UInt2 GetSize() const;
        [[nodiscard]] UInt2 GetSizeInPixels() const;
        [[nodiscard]] float GetAspectRatio() const;

#if TODO
        [[nodiscard]] Int2 GetPosition() const;
        void SetPosition(int32_t x, int32_t y) noexcept;
        void SetPosition(const Int2& position) noexcept;

        
        void SetSize(int32_t width, int32_t height) noexcept;
        void SetSize(const SizeI& size) noexcept;
#endif // TODO


        [[nodiscard]] const std::string& GetTitle() const { return _title; }
        void SetTitle(std::string_view title);

        void Show();
        void Hide();
        void Maximize();
        void Minimize();
        void Restore();

        [[nodiscard]] bool IsOpen() const;
        [[nodiscard]] bool IsMinimized() const;
        [[nodiscard]] bool IsFullscreen() const;
        [[nodiscard]] bool IsFocused() const;
        [[nodiscard]] bool IsCursorVisible() const;

        void RequestFocus();
        void SetFullscreen(bool value);
        void SetCursorVisible(bool value);

        /// Creates a SwapChain using the provided RHIDevice
        void CreateSwapChain(RHIDevice* device);

        [[nodiscard]] WindowImpl* GetImpl() const noexcept { return _impl; }
        [[nodiscard]] RHISurface* GetSurface() const noexcept { return _surface.Get(); }
        PixelFormat GetColorFormat() const noexcept { return _colorFormat; }

    private:
        /* Called by Application */
        void CreateSurface(RHIFactory* factory);
        void DestroySwapChain();
        void OnResized();

        WindowImpl* _impl;
        uint32_t _id{};
        std::string _title;
        PixelFormat _colorFormat = PixelFormat::BGRA8UnormSrgb;
        RHISurfaceRef _surface;
    };
}
