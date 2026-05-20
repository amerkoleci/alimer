// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_USE_SDL)
#include "Alimer/Platform/SDL/Window.SDL.h"
#endif
#include "Alimer/Core/Log.h"

using namespace Alimer;

void Window::CreateSwapChain(RHIDevice* device)
{
    ALIMER_ASSERT(device);

    UInt2 size = GetSizeInPixels();

    const RHISurfaceConfig config{
        .label = "Window SwapChain",
        .format = _colorFormat,
        .width = size.x,
        .height = size.y,
        .presentMode = RHIPresentMode::Fifo,
    };
    _surface->Configure(device, config);
}

void Window::OnResized()
{
    if (_surface != nullptr)
    {
        UInt2 size = GetSize();
        _surface->Resize(size.x, size.y);
    }

    Resized.Emit(this);
}

void Window::DestroySwapChain()
{
    _surface.Reset();
}

float Window::GetAspectRatio() const
{
    auto size = GetSize();
    if (size.x == 0 || size.y == 0)
        return 0.0f;

    return static_cast<float>(size.x) / size.y;
}

#if TODO
void Window::SetPosition(const Int2& position) noexcept
{
    SetPosition(position.x, position.y);
}

void Window::SetSize(const SizeI& size) noexcept
{
    SetSize(size.width, size.height);
}

#endif // TODO
