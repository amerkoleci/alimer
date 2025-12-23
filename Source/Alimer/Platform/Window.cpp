// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_USE_SDL)
#include "Alimer/Platform/SDL/Window.SDL.h"
#endif
#include "Alimer/Core/Log.h"

using namespace Alimer;

void Window::OnResized()
{
    if (_swapChain != nullptr)
    {
        _device->WaitIdle();

        //CreateSwapChain();
    }

    Resized.Emit();
}

void Window::CreateSwapChain(RHIDevice* device)
{
    _device.Reset(device);
    UInt2 size = GetSize();

    const RHISwapChainDesc desc{
        .label = "Window SwapChain",
        .width = size.x,
        .height = size.y,
        .colorFormat = _colorFormat,
        .presentMode = PresentMode::Fifo,
    };
    _swapChain = _device->CreateSwapChain(_surface, desc);
}

void Window::DestroySwapChain()
{
    _device->WaitIdle();
    _swapChain.Reset();
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
