// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"
#include "alimer_platform.h"
//#if defined(ALIMER_GPU)
//#include "alimer_gpu.h"
//#endif

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/dom_pk_codes.h>

#include <string>
#include <deque>

namespace
{
}

struct Window
{
    std::string selector;
    uint32_t id = 0;
};

static struct {
    bool initialized;
    std::string clipboard;
    std::deque<PlatformEvent> event_queue;
} state;

static void PushEvent(PlatformEvent&& e)
{
    state.event_queue.emplace_back(std::move(e));
}

static bool PopEvent(PlatformEvent* e) noexcept
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

bool alimerPlatformInit(void)
{
    if (state.initialized)
        return true;

    state.initialized = true;
    return true;
}

void alimerPlatformShutdown(void)
{
    if (!state.initialized)
        return;

    memset(&state, 0, sizeof(state));
}

bool alimerPlatformPollEvent(PlatformEvent* evt)
{
    ALIMER_ASSERT(state.initialized);

    //SDL_Event ev{};
    //while (SDL_PollEvent(&ev) != 0)
    //{
    //    auto e = ToEvent(ev);
    //    PushEvent(std::move(e));
    //}

    return PopEvent(evt);
}

void alimerPlatformGetMousePosition(float* x, float* y)
{
}

Window* alimerWindowCreate(const WindowDesc* desc)
{
    ALIMER_ASSERT(desc);

    const bool fullscreen = desc->flags & WindowFlags_Fullscreen;

    Window* window = new Window();
    window->selector = "#canvas";
    window->id = 0;
    return window;
}

void alimerWindowDestroy(Window* window)
{
    delete window;
}

uint32_t alimerWindowGetID(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    return window->id;
}

bool alimerWindowIsOpen(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    return false;
}

void alimerWindowSetPosition(Window* window, int32_t x, int32_t y)
{
    ALIMER_ASSERT(window != nullptr);
}

void alimerWindowGetPosition(Window* window, int32_t* x, int32_t* y)
{
    ALIMER_ASSERT(window != nullptr);
}

void alimerWindowSetCentered(Window* window)
{
    ALIMER_ASSERT(window != nullptr);
}

void alimerWindowSetSize(Window* window, uint32_t width, uint32_t height)
{
    ALIMER_ASSERT(window != nullptr);
}

void alimerWindowGetSize(Window* window, uint32_t* width, uint32_t* height)
{
    ALIMER_ASSERT(window != nullptr);
}

void alimerWindowGetSizeInPixels(Window* window, uint32_t* width, uint32_t* height)
{
    ALIMER_ASSERT(window != nullptr);
}

void alimerWindowSetTitle(Window* window, const char* title)
{
    ALIMER_ASSERT(window != nullptr);
}

const char* alimerWindowGetTitle(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    return nullptr;
}

float alimerWindowGetDisplayScale(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    const double ratio = emscripten_get_device_pixel_ratio();
    return ratio > 0.0 ? static_cast<float>(ratio) : 1.0f;
}

bool alimerWindowGetMousePosition(Window* window, float* x, float* y)
{
    ALIMER_ASSERT(window != nullptr);

    return false;
}

bool alimerWindowIsMinimized(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    return false;
}

bool alimerWindowIsMaximized(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    return false;
}

bool alimerWindowIsFullscreen(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    return false;
}

void alimerWindowSetFullscreen(Window* window, bool value)
{
    ALIMER_ASSERT(window != nullptr);
}

bool alimerWindowHasFocus(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    return false;
}

void alimerWindowShow(Window* window)
{
    ALIMER_ASSERT(window != nullptr);
}

void alimerWindowHide(Window* window)
{
    ALIMER_ASSERT(window != nullptr);
}

void alimerWindowMaximize(Window* window)
{
    ALIMER_ASSERT(window != nullptr);
}

void alimerWindowMinimize(Window* window)
{
    ALIMER_ASSERT(window != nullptr);
}

void alimerWindowRestore(Window* window)
{
    ALIMER_ASSERT(window != nullptr);
}

void alimerWindowFocus(Window* window)
{
    ALIMER_ASSERT(window != nullptr);
}

void* alimerWindowGetNativeDisplay(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    return nullptr;
}

void* alimerWindowGetNativeHandle(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    return nullptr;
}

/* Clipboard */
bool alimerHasClipboardText(void)
{
    return !state.clipboard.empty();
}

const char* alimerClipboardGetText(void)
{
    return state.clipboard.c_str();
}

void alimerClipboardSetText(const char* text)
{
    state.clipboard = text;
}

/* PowerStatus */
PowerLineStatus alimerGetPowerLineStatus(void)
{
    return PowerLineStatus_Unknown;
}

BatteryStatus alimerGetBatteryStatus(int* batteryLifeTime, int* batteryLifePercent)
{
    return BatteryStatus_Unknown;
}
