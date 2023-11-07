// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_USE_GLFW)
#include "alimer_internal.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

//#if defined(_WIN32)
//#define GLFW_EXPOSE_NATIVE_WIN32
//#elif defined(__APPLE__)
//#define GLFW_EXPOSE_NATIVE_COCOA
//#include <Foundation/Foundation.h>
//#include <QuartzCore/CAMetalLayer.h>
//#define GLFW_EXPOSE_NATIVE_X11
//#include <X11/Xlib-xcb.h>
//#endif
//#include <GLFW/glfw3native.h>

static void OnGLFWError(int code, const char* description)
{
    Alimer_LogError("GLFW error %d: %s", code, description);
}

static AlimerState state;

AlimerState* GetState()
{
    return &state;
}

Bool32 Alimer_Init(const Config* config)
{
    ALIMER_ASSERT(config);

    if (state.initialized)
        return false;

    glfwSetErrorCallback(OnGLFWError);
#ifdef __APPLE__
    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
    if (!glfwInit()) {
        return false;
    }

    // Get SDL version
    int major, minor, patch;
    glfwGetVersion(&major, &minor, &patch);
    Alimer_LogInfo("GLFW v%d.%d.%d", major, minor, patch);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    state.initialized = true;

    return true;
}

void Alimer_Shutdown(void)
{
    if (!state.initialized)
        return;

    glfwTerminate();

    memset(&state, 0, sizeof(state));
}


#endif /* defined(ALIMER_USE_GLFW) */
