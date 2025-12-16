// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/Platform/Window.h"

using namespace Alimer;

#if TODO
void Window::OnResized()
{
    if (swapChain != nullptr)
    {
        GRHIDevice->WaitIdle();
    }

    //swapChain->Resize();
}

void Window::CreateSwapChain()
{
    if (swapChain != nullptr)
    {
        GRHIDevice->WaitIdle();
    }

    auto size = GetSize();

    RHISwapChainDesc desc{};
    desc.width = size.width;
    desc.height = size.height;
    //desc.usage = colorUsage;
    desc.colorFormat = colorFormat;
    desc.presentMode = PresentMode::Fifo;
    //desc.presentMode = PresentMode::Immediate;
    swapChain = RHICreateSwapChain(surface, desc);
}

void Window::SetPosition(const Int2& position) noexcept
{
    SetPosition(position.x, position.y);
}

void Window::SetSize(const SizeI& size) noexcept
{
    SetSize(size.width, size.height);
}

float Window::GetAspectRatio() const
{
    auto size = GetSize();
    if (size.width == 0 || size.height == 0)
        return 0.0f;

    return static_cast<float>(size.width) / size.height;
}

#endif // TODO
