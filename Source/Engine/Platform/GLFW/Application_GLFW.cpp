// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Application.h"
//#include "Platform/Window.h"

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
            //LOGE("GLFW error {}: {}", code, description);
        }

        GLFWwindow* window = nullptr;
    }

    bool Application::PlatformInit()
    {
        glfwSetErrorCallback(OnGLFWError);
#ifdef __APPLE__
        glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
        if (glfwInit() != GLFW_TRUE)
        {
            return false;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        return true;
    }

    void Application::PlatformShutdown()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool Application::PlatformSetup(const Settings& settings)
    {
        glfwWindowHint(GLFW_RESIZABLE, settings.resizable);

        GLFWmonitor* monitor = nullptr;
        if (settings.fullscreen)
        {
            auto mode = glfwGetVideoMode(monitor);

            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        }

        window = glfwCreateWindow(settings.width, settings.height, settings.title.c_str(), monitor, nullptr);

        if (!window)
        {
            //LOGE("Failed to create GLFW window!");
            return false;
        }

        //glfwSetKeyCallback(window, key_callback_glfw);
        //glfwSetCursorPosCallback(window, mouse_callback_glfw);
        //glfwSetScrollCallback(window, scroll_callback_glfw);
        //glfwSetMouseButtonCallback(window, mouse_button_callback_glfw);
        //glfwSetCharCallback(window, char_callback_glfw);
        //glfwSetWindowSizeCallback(window, window_size_callback_glfw);
        //glfwSetWindowIconifyCallback(window, window_iconify_callback_glfw);
        glfwSetWindowUserPointer(window, this);

        //LOGI("Successfully initialized platform!");
        return true;
    }

    void Application::PlatformUpdate()
    {
        glfwPollEvents();
    }

    void Application::RequestExit()
    {
        glfwSetWindowShouldClose(window, true);
    }

    bool Application::IsExitRequested() const
    {
        return glfwWindowShouldClose(window) == GLFW_TRUE;
    }

    void* Application::GetWindowHandle() const
    {
        assert(window);

#if defined(GLFW_EXPOSE_NATIVE_WIN32)
        return glfwGetWin32Window(window);
#elif defined(GLFW_EXPOSE_NATIVE_X11)
        return glfwGetX11Window(window);
#elif defined(GLFW_EXPOSE_NATIVE_COCOA)
        return glfwGetCocoaWindow(window);
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND)
        return glfwGetWaylandWindow(window);
#else
        return nullptr;
#endif
    }
}
