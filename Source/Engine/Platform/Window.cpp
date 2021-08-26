// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Window.h"

namespace alimer
{
    Window::Window(const std::string_view& title, int32_t width, int32_t height, WindowFlags flags)
        : Window(title, Int2{ Centered, Centered }, Int2{ width, height }, flags)
    {

    }

    Window::Window(const std::string_view& title, int32_t x, int32_t y, int32_t width, int32_t height, WindowFlags flags)
        : Window(title, Int2{ x, y }, Int2{ width, height }, flags)
    {
    }

    Window::Window(const std::string_view& title, const Int2& size, WindowFlags flags)
        : Window(title, Int2{ Centered, Centered }, size, flags)
    {

    }

    void Window::OnClosed()
    {
        ALIMER_ASSERT(isClosing);
        //depthStencilTexture.Reset();
        //swapChain.Reset();

        Closed(this);
    }
}
