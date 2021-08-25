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
    }

    void Application::PlatformInit()
    {
        glfwSetErrorCallback(OnGLFWError);
#ifdef __APPLE__
        glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
        if (!glfwInit())
        {
            return;
        }
    }

    void Application::PlatformShutdown()
    {
        glfwTerminate();
    }

    void Application::PlatformRun()
    {

    }
}
