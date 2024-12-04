// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_PLATFORM_GLFW)
#include "alimer_internal.h"
#include "alimer.h"
//#if defined(ALIMER_GPU_VULKAN)
//#define VK_NO_PROTOTYPES
//#include <vulkan/vulkan.h>
//#endif
#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>
#elif defined(__APPLE__) 
#include <Foundation/Foundation.h>
#include <QuartzCore/CAMetalLayer.h>
#include <GLFW/glfw3native.h>
#endif

#include <vector>
#include <deque>

static void OnGLFWError(int code, const char* description)
{
    alimerLogError(LogCategory_Application, "GLFW error %d: %s", code, description);
}

static void MonitorCallback(GLFWmonitor* monitor, int event)
{
    ALIMER_UNUSED(monitor);
    ALIMER_UNUSED(event);
}

static ButtonState FromGLFWState(int id)
{
    switch (id)
    {
        case GLFW_RELEASE:
            return ButtonState_Released;
        case GLFW_PRESS:
            return ButtonState_Pressed;

        default:
            return ButtonState_None;
    }
}


static MouseButton FromGLFW(int id)
{
    switch (id)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
            return MouseButton_Left;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return MouseButton_Right;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return MouseButton_Middle;
        case GLFW_MOUSE_BUTTON_4:
            return MouseButton_X1;
        case GLFW_MOUSE_BUTTON_5:
            return MouseButton_X2;
        default:
            return MouseButton_None;
    }
}

struct Window
{
    GLFWwindow* handle;
    uint32_t id;
    char* title;
};

static struct {
    bool initialized;
    std::vector<Window*> windows;
    std::deque<Event> event_queue;
    Window* focusedWindow = nullptr;
} state;

static uint32_t RegisterWindow(Window* window)
{
    static uint32_t id{ 0 };
    state.windows.emplace_back(window);
    return ++id;
}

static void UnregisterWindow(uint32_t id)
{
    for (size_t i = 0, count = state.windows.size(); i < count; ++i)
    {
        if (state.windows[i]->id == id)
        {
            state.windows.erase(state.windows.begin() + i);
            break;
        }
    }
}

static bool glfwSetWindowCenter(GLFWwindow* window)
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
        int minX = std::max(mx, px);
        int minY = std::max(my, py);

        int maxX = std::min(mx + mode->width, px + sx);
        int maxY = std::min(my + mode->height, py + sy);

        // Calculate area of the intersection
        int area = std::max(maxX - minX, 0) * std::max(maxY - minY, 0);

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
    {
        glfwSetWindowPos(window, final_x, final_y);
    }
    else
    {
        // Something is wrong - current window has NOT any intersection with any monitors. Move it to the default
        // one.
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

static void PushEvent(Event&& e)
{
    state.event_queue.emplace_back(std::move(e));
}

static bool PopEvent(Event* e) noexcept
{
    // Pop the first event of the queue, if it is not empty
    if (!state.event_queue.empty())
    {
        *e = state.event_queue.front();
        state.event_queue.pop_front();
        return true;
    }

    return false;
}

static Window* GetImpl(GLFWwindow* window)
{
    return reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
}

static void OnWindowClose(GLFWwindow* window)
{
    Window* impl = GetImpl(window);

    Event evt{};
    evt.type = EventType_Window;
    evt.window.windowId = impl->id;
    evt.window.type = WindowEventType_Close;
    PushEvent(std::move(evt));
}

static void OnWindowFocus(GLFWwindow* window, int focused)
{
    Window* impl = GetImpl(window);

    if (focused == GL_TRUE)
    {
        state.focusedWindow = impl;
    }
    else
    {
        // only if this was the focused one
        if (state.focusedWindow == impl)
        {
            state.focusedWindow = nullptr;
        }
    }

    Event evt{};
    evt.type = EventType_Window;
    evt.window.windowId = impl->id;
    evt.window.type = focused == GL_TRUE ? WindowEventType_FocusGained : WindowEventType_FocusLost;
    PushEvent(std::move(evt));
}

static void OnWindowResize(GLFWwindow* window, int w, int h)
{
    Window* impl = GetImpl(window);

    Event evt{};
    evt.type = EventType_Window;
    evt.window.windowId = impl->id;
    evt.window.type = WindowEventType_Resized;
    evt.window.data1 = static_cast<int32_t>(w);
    evt.window.data2 = static_cast<int32_t>(h);
    PushEvent(std::move(evt));
}

static void OnWindowPositionChanged(GLFWwindow* window, int x, int y)
{
    Window* impl = GetImpl(window);

    Event evt{};
    evt.type = EventType_Window;
    evt.window.windowId = impl->id;
    evt.window.type = WindowEventType_Moved;
    evt.window.data1 = static_cast<int32_t>(x);
    evt.window.data2 = static_cast<int32_t>(y);
    PushEvent(std::move(evt));
}

static void OnWindowMaximized(GLFWwindow* window, int mode)
{
    Window* impl = GetImpl(window);

    Event evt{};
    evt.type = EventType_Window;
    evt.window.windowId = impl->id;
    evt.window.type = mode == GLFW_TRUE ? WindowEventType_Maximized : WindowEventType_Restored;
    PushEvent(std::move(evt));
}

static void OnWindowMinimized(GLFWwindow* window, int mode)
{
    Window* impl = GetImpl(window);

    Event evt{};
    evt.type = EventType_Window;
    evt.window.windowId = impl->id;
    evt.window.type = mode == GLFW_TRUE ? WindowEventType_Minimized : WindowEventType_Restored;
    PushEvent(std::move(evt));
}

static void OnWindowCursorEnter(GLFWwindow* window, int mode)
{
    Window* impl = GetImpl(window);

    Event evt{};
    evt.type = EventType_Window;
    evt.window.windowId = impl->id;
    evt.window.type = mode == GLFW_TRUE ? WindowEventType_Enter : WindowEventType_Leave;
    PushEvent(std::move(evt));
}

static void OnWindowMouseMove(GLFWwindow* window, double x, double y)
{
    Window* impl = GetImpl(window);

    Event evt{};
    evt.type = EventType_MouseMotion;
    evt.motion.windowId = impl->id;
    evt.motion.x = static_cast<int32_t>(x);
    evt.motion.y = static_cast<int32_t>(y);
    PushEvent(std::move(evt));
}

static void OnWindowMouseButton(GLFWwindow* window, int button, int action, int)
{
    Window* impl = GetImpl(window);

    double x{};
    double y{};
    glfwGetCursorPos(window, &x, &y);

    Event evt{};
    evt.type = EventType_MouseButton;
    evt.button.windowId = impl->id;
    evt.button.button = FromGLFW(button);
    evt.button.state = FromGLFWState(action);
    evt.button.x = static_cast<float>(x);
    evt.button.y = static_cast<float>(y);

    PushEvent(std::move(evt));
}

static void OnWindowMouseWheelMove(GLFWwindow* window, double xoffs, double yoffs)
{
    Window* impl = GetImpl(window);

    Event evt{};
    evt.type = EventType_MouseWheel;
    evt.wheel.windowId = impl->id;
    evt.wheel.x = static_cast<float>(xoffs);
    evt.wheel.y = static_cast<float>(yoffs);

    PushEvent(std::move(evt));
}

bool alimerPlatformInit(void)
{
    if (state.initialized)
        return true;

    // Init glfw
    glfwSetErrorCallback(OnGLFWError);
#ifdef __APPLE__
    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
    if (!glfwInit())
    {
        alimerLogError(LogCategory_Application, "glfwInit Failed");
        return false;
    }

    // Get SDL version
    int major, minor, patch;
    glfwGetVersion(&major, &minor, &patch);
    alimerLogInfo(LogCategory_Application, "GLFW v%d.%d.%d", major, minor, patch);

    glfwSetMonitorCallback(MonitorCallback);

    state.initialized = true;
    return true;
}

void alimerPlatformShutdown(void)
{
    if (!state.initialized)
        return;

    glfwTerminate();
    memset(&state, 0, sizeof(state));
}

bool alimerPollEvent(Event* evt)
{
    ALIMER_ASSERT(state.initialized);

    // Pump events
    glfwPollEvents();

    static bool reported = false;
    if (!reported)
    {
        bool allClosed = true;
        for (size_t i = 0, count = state.windows.size(); i < count; ++i)
        {
            if (!glfwWindowShouldClose(state.windows[i]->handle))
            {
                allClosed = false;
                break;
            }
        }

        if (allClosed)
        {
            reported = true;

            Event evt{};
            evt.type = EventType_Quit;
            PushEvent(std::move(evt));
        }
    }

    return PopEvent(evt);
}

Window* alimerWindowCreate(const WindowDesc* desc)
{
    ALIMER_ASSERT(desc);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const int visible = desc->flags & WindowFlags_Hidden ? GLFW_FALSE : GLFW_TRUE;
    glfwWindowHint(GLFW_VISIBLE, visible);

    const int decorated = desc->flags & WindowFlags_Borderless ? GLFW_FALSE : GLFW_TRUE;
    glfwWindowHint(GLFW_DECORATED, decorated);

    const int resizable = desc->flags & WindowFlags_Resizable ? GLFW_TRUE : GLFW_FALSE;
    glfwWindowHint(GLFW_RESIZABLE, resizable);

    const int maximized = desc->flags & WindowFlags_Maximized ? GLFW_TRUE : GLFW_FALSE;
    glfwWindowHint(GLFW_MAXIMIZED, maximized);

    const int on_top = desc->flags & WindowFlags_AlwaysOnTop ? GLFW_TRUE : GLFW_FALSE;
    glfwWindowHint(GLFW_FLOATING, on_top);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    uint32_t width = desc->width ? desc->width : (uint32_t)mode->width;
    uint32_t height = desc->height ? desc->height : (uint32_t)mode->height;

    bool fullscreen = desc->flags & WindowFlags_Fullscreen;

    // fullscreen_desktop
    if (fullscreen)
    {
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }

    GLFWwindow* window = glfwCreateWindow(width, height, desc->title, fullscreen ? monitor : nullptr, nullptr);
    if (!window)
    {
        return nullptr;
    }

    glfwDefaultWindowHints();

    if (desc->icon.data)
    {
        GLFWimage image{};
        image.width = desc->icon.width;
        image.height = desc->icon.height;
        image.pixels = desc->icon.data;
        glfwSetWindowIcon(window, 1, &image);
    }

    Window* result = ALIMER_ALLOC(Window);
    result->handle = window;
    result->id = RegisterWindow(result);
    result->title = _alimer_strdup(desc->title);

    glfwSetWindowUserPointer(result->handle, result);
    glfwSetWindowCloseCallback(result->handle, OnWindowClose);
    glfwSetWindowFocusCallback(result->handle, OnWindowFocus);
    glfwSetWindowSizeCallback(result->handle, OnWindowResize);
    glfwSetWindowPosCallback(result->handle, OnWindowPositionChanged);
    glfwSetWindowMaximizeCallback(result->handle, OnWindowMaximized);
    glfwSetWindowIconifyCallback(result->handle, OnWindowMinimized);
    glfwSetCursorEnterCallback(result->handle, OnWindowCursorEnter);
    glfwSetCursorPosCallback(result->handle, OnWindowMouseMove);
    glfwSetMouseButtonCallback(result->handle, OnWindowMouseButton);
    glfwSetScrollCallback(result->handle, OnWindowMouseWheelMove);
    //glfwSetKeyCallback(result->handle, onKeyboardEvent);
    //glfwSetCharCallback(result->handle, onTextEvent);

    if (alimerWindowHasFocus(result))
    {
        state.focusedWindow = result;
    }

    return result;
}

void alimerWindowDestroy(Window* window)
{
    UnregisterWindow(window->id);
    glfwDestroyWindow(window->handle);
    alimerFree(window->title);
    alimerFree(window);
}

uint32_t alimerWindowGetID(Window* window)
{
    return window->id;
}

bool alimerWindowIsOpen(Window* window)
{
    return !glfwWindowShouldClose(window->handle);
}

void alimerWindowSetPosition(Window* window, int32_t x, int32_t y)
{
    glfwSetWindowPos(window->handle, x, y);
}

void alimerWindowGetPosition(Window* window, int32_t* x, int32_t* y)
{
    glfwGetWindowPos(window->handle, x, y);
}

void alimerWindowSetCentered(Window* window)
{
    glfwSetWindowCenter(window->handle);
}

void alimerWindowSetSize(Window* window, uint32_t width, uint32_t height)
{
    glfwSetWindowSize(window->handle, static_cast<int>(width), static_cast<int>(height));
}

void alimerWindowGetSize(Window* window, uint32_t* width, uint32_t* height)
{
    int w{};
    int h{};
    glfwGetWindowSize(window->handle, &w, &h);
    if (width)
        *width = static_cast<uint32_t>(w);
    if (height)
        *height = static_cast<uint32_t>(h);
}

void alimerWindowSetTitle(Window* window, const char* title)
{
    char* prev = window->title;
    window->title = _alimer_strdup(title);

    glfwSetWindowTitle(window->handle, title);
    alimerFree(prev);
}

const char* alimerWindowGetTitle(Window* window)
{
    return window->title;
}

bool alimerWindowIsMinimized(Window* window)
{
    return glfwGetWindowAttrib(window->handle, GLFW_ICONIFIED) != 0;
}

bool alimerWindowHasFocus(Window* window)
{
    return glfwGetWindowAttrib(window->handle, GLFW_FOCUSED) != 0;
}

void alimerWindowShow(Window* window)
{
    glfwShowWindow(window->handle);
}

void alimerWindowHide(Window* window)
{
    glfwHideWindow(window->handle);
}

void alimerWindowMaximize(Window* window)
{
    glfwMaximizeWindow(window->handle);
}

void alimerWindowMinimize(Window* window)
{
    glfwIconifyWindow(window->handle);
}

void alimerWindowRestore(Window* window)
{
    glfwRestoreWindow(window->handle);
}

void alimerWindowRaise(Window* window)
{
    glfwRequestWindowAttention(window->handle);
}

void alimerWindowRequestFocus(Window* window)
{
    glfwFocusWindow(window->handle);
}

void* alimerWindowGetNativeHandle(Window* window)
{
#if defined(GLFW_EXPOSE_NATIVE_COCOA)
    return glfwGetCocoaWindow(window->handle);
#elif defined(GLFW_EXPOSE_NATIVE_WAYLAND) && defined(GLFW_EXPOSE_NATIVE_X11)
    if (glfwGetPlatform() == GLFW_PLATFORM_X11)
    {
        return (void*)(uintptr_t)glfwGetX11Window(window->handle);
    }
    else if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND)
    {
        return glfwGetWaylandWindow(window->handle);
    }
#elif defined(GLFW_EXPOSE_NATIVE_WIN32)
    return glfwGetWin32Window(window->handle);
#else
    return nullptr;
#endif
}

/* Clipboard */
void alimerClipboardSetText(const char* text)
{
    glfwSetClipboardString(nullptr, text);
}

const char* alimerClipboardGetText(void)
{
    return glfwGetClipboardString(nullptr);
}

#endif /* defined(ALIMER_PLATFORM_GLFW) */
