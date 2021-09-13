// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_PLATFORM_GLFW)
#include "Application.h"
//#include "Platform/Window.h"
#include "Core/Log.h"

#define GLFW_INCLUDE_NONE
#if ALIMER_PLATFORM_WINDOWS
#   define GLFW_EXPOSE_NATIVE_WIN32
#elif ALIMER_PLATFORM_MACOS
#   define GLFW_EXPOSE_NATIVE_COCOA
#elif ALIMER_PLATFORM_LINUX
#   define GLFW_EXPOSE_NATIVE_X11
//# define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Alimer
{
    namespace
    {
        static inline void OnGLFWError(int code, const char* description)
        {
            LOGE("GLFW error {}: {}", code, description);
        }

        uint32_t windowCount = 0;

        inline bool glfwSetWindowCenter(GLFWwindow* window)
        {
            if (!window)
                return false;

            int sx = 0, sy = 0;
            int px = 0, py = 0;
            int mx = 0, my = 0;
            int monitor_count = 0;
            int best_area = 0;
            int final_x = 0, final_y = 0;

            glfwGetWindowSize(window, &sx, &sy);
            glfwGetWindowPos(window, &px, &py);

            // Iterate throug all monitors
            GLFWmonitor** m = glfwGetMonitors(&monitor_count);
            if (!m)
                return false;

            for (int j = 0; j < monitor_count; ++j)
            {

                glfwGetMonitorPos(m[j], &mx, &my);
                const GLFWvidmode* mode = glfwGetVideoMode(m[j]);
                if (!mode)
                    continue;

                // Get intersection of two rectangles - screen and window
                int minX = Max(mx, px);
                int minY = Max(my, py);

                int maxX = Min(mx + mode->width, px + sx);
                int maxY = Min(my + mode->height, py + sy);

                // Calculate area of the intersection
                int area = Max(maxX - minX, 0) * Max(maxY - minY, 0);

                // If its bigger than actual (window covers more space on this monitor)
                if (area > best_area)
                {
                    // Calculate proper position in this monitor
                    final_x = mx + (mode->width - sx) / 2;
                    final_y = my + (mode->height - sy) / 2;

                    best_area = area;
                }
            }

            // We found something
            if (best_area)
                glfwSetWindowPos(window, final_x, final_y);

            // Something is wrong - current window has NOT any intersection with any monitors. Move it to the default
            // one.
            else
            {
                GLFWmonitor* primary = glfwGetPrimaryMonitor();
                if (primary)
                {
                    const GLFWvidmode* desktop = glfwGetVideoMode(primary);

                    if (desktop)
                        glfwSetWindowPos(window, (desktop->width - sx) / 2, (desktop->height - sy) / 2);
                    else
                        return false;
                }
                else
                    return false;
            }

            return true;
        }
    }

    struct WindowImpl final
    {
        GLFWwindow* window{ nullptr };
    };

    void Application::PlatformConstruct()
    {
        glfwSetErrorCallback(OnGLFWError);
#ifdef __APPLE__
        glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
        if (glfwInit() != GLFW_TRUE)
        {
            LOGE("Failed to initialize GLFW");
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    void Application::PlatformShutdown()
    {
        glfwTerminate();
    }

    bool Application::PlatformSetup(const Settings& settings)
    {
        WindowFlags windowFlags = WindowFlags::None;
        if (settings.fullscreen)
        {
            windowFlags = WindowFlags::Fullscreen;
        }
        else
        {
            if (settings.resizable)
            {
                windowFlags |= WindowFlags::Resizable;
            }
        }

        window = std::make_unique<Window>(settings.title, settings.width, settings.height, windowFlags);

        LOGI("Successfully initialized platform!");
        return true;
    }

    void Application::PlatformUpdate()
    {
        glfwPollEvents();
    }

    /* Window */
    Window::Window(const std::string_view& title_, const Int2& position, const Int2& size, WindowFlags flags)
        : title(title_)
        , impl(std::make_unique<WindowImpl>())
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        auto decorated = CheckBitsAny(flags, WindowFlags::Borderless) ? GLFW_FALSE : GLFW_TRUE;
        glfwWindowHint(GLFW_DECORATED, decorated);

        auto resizable = CheckBitsAny(flags, WindowFlags::Resizable) ? GLFW_TRUE : GLFW_FALSE;
        glfwWindowHint(GLFW_RESIZABLE, resizable);

        auto maximized = CheckBitsAny(flags, WindowFlags::Maximized) ? GLFW_TRUE : GLFW_FALSE;
        glfwWindowHint(GLFW_MAXIMIZED, maximized);

        GLFWmonitor* monitor = nullptr;
        if (CheckBitsAny(flags, WindowFlags::Fullscreen))
        {
            monitor = glfwGetPrimaryMonitor();
        }

        if (CheckBitsAny(flags, WindowFlags::FullscreenDesktop))
        {
            monitor = glfwGetPrimaryMonitor();
            auto mode = glfwGetVideoMode(monitor);

            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        }

        impl->window = glfwCreateWindow(size.x, size.y, title.data(), monitor, nullptr);
        if (!impl->window)
        {
            LOGE("GLFW: Failed to create window");
            return;
        }

        windowCount++;

        glfwDefaultWindowHints();
        glfwSetWindowUserPointer(impl->window, this);
        glfwSetWindowCloseCallback(impl->window, [](GLFWwindow* glfwWindow) {
            Window* engineWindow = (Window*)glfwGetWindowUserPointer(glfwWindow);
            engineWindow->Close();
            }
        );

        SetPosition(position);
    }

    Window::~Window()
    {
        Close();
        Destroy();
    }

    void Window::Destroy()
    {
        if (impl->window)
        {
            glfwDestroyWindow(impl->window);
            impl->window = nullptr;
        }

        //windowCount--;
    }

    void Window::Show()
    {
        ALIMER_ASSERT(impl->window);

        if (!swapChain)
        {
            CreateSwapChain();
        }

        glfwShowWindow(impl->window);
    }

    void Window::Hide()
    {
        ALIMER_ASSERT(impl->window);

        glfwHideWindow(impl->window);
    }

    void Window::Close()
    {
        if (isClosing)
            return;

        isClosing = true;
        bool cancelClose = false;
        Closing(this, cancelClose);
        if (cancelClose)
        {
            isClosing = false;
            return;
        }

        if (impl->window)
        {
            glfwSetWindowShouldClose(impl->window, GLFW_TRUE);
        }

        OnClosed();
    }

    bool Window::ShouldClose() const
    {
        return glfwWindowShouldClose(impl->window) == GLFW_TRUE;
    }

    bool Window::IsMinimized() const
    {
        if (glfwWindowShouldClose(impl->window) == GLFW_TRUE)
            return true;

        int width = 0, height = 0;
        glfwGetFramebufferSize(impl->window, &width, &height);
        return width == 0 || height == 0;
    }

    void Window::SetTitle(const std::string_view& newTitle)
    {
        title = newTitle;
        glfwSetWindowTitle(impl->window, newTitle.data());
    }

    void Window::SetPosition(const Int2& pos)
    {
        if (pos.x == Window::Centered && pos.y == Window::Centered)
        {
            glfwSetWindowCenter(impl->window);
        }
        else
        {
            glfwSetWindowPos(impl->window, static_cast<int>(pos.x), static_cast<int>(pos.y));
        }
    }

    Int2 Window::GetPosition() const
    {
        Int2 size;
        glfwGetWindowPos(impl->window, &size.x, &size.y);
        return size;
    }

    Int2 Window::GetSize() const
    {
        Int2 size;
        glfwGetWindowSize(impl->window, &size.x, &size.y);
        return size;
    }

    void Window::SetSize(const Int2& size)
    {
        glfwSetWindowSize(impl->window, static_cast<int>(size.x), static_cast<int>(size.y));
    }

    void* Window::GetPlatformHandle() const
    {
        ALIMER_ASSERT(impl->window);

#if defined(GLFW_EXPOSE_NATIVE_WIN32)
        return glfwGetWin32Window(impl->window);
#elif defined(GLFW_EXPOSE_NATIVE_X11)
        return glfwGetX11Window(impl->window);
#elif defined(GLFW_EXPOSE_NATIVE_COCOA)
        return glfwGetCocoaWindow(impl->window);
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND)
        return glfwGetWaylandWindow(impl->window);
#else
        return nullptr;
#endif
    }
}

#endif /* defined(ALIMER_PLATFORM_GLFW) */
