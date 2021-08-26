// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Types.h"
#include "Core/Signal.h"

namespace alimer
{
    enum class WindowFlags
    {
        None = 0,
        Fullscreen = 1 << 0,
        FullscreenDesktop = 1 << 1,
        Borderless = 1 << 2,
        Resizable = 1 << 3,
        Minimized = 1 << 4,
        Maximized = 1 << 5,
    };
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(WindowFlags);

    struct Int2
    {
        int32_t x;
        int32_t y;
    };

    struct WindowImpl;

    /// Defines OS window.
    class ALIMER_API Window final
    {
    public:
        constexpr static const int32_t Centered = std::numeric_limits<int32_t>::max();

        /// Occurs directly after Close() is called, and can be handled to cancel window closure.
        Signal<Window*, bool&> Closing;

        /// Occurs when the window is about to close.
        Signal<Window*> Closed;

        Window(const std::string_view& title, int32_t width, int32_t height, WindowFlags flags = WindowFlags::None);
        Window(const std::string_view& title, int32_t x, int32_t y, int32_t width, int32_t height, WindowFlags flags = WindowFlags::None);
        Window(const std::string_view& title, const Int2& size, WindowFlags flags = WindowFlags::None);
        Window(const std::string_view& title, const Int2& position, const Int2& size, WindowFlags flags = WindowFlags::None);

        ~Window();

        void Show();
        void Hide();
        void Close();
        [[nodiscard]] bool ShouldClose() const;
        [[nodiscard]] void* GetPlatformHandle() const;

        [[nodiscard]] std::string GetTitle() const noexcept { return title; }
        void SetTitle(const std::string_view& newTitle);

        [[nodiscard]] bool IsMinimized() const;

        [[nodiscard]] WindowImpl* GetImpl() const { return impl.get(); }

    private:
        ALIMER_DISABLE_COPY_MOVE(Window);

        void Destroy();
        void OnClosed();

        bool isClosing{ false };
        std::string title;

        std::unique_ptr<WindowImpl> impl;
    };
}

