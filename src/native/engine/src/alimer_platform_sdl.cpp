// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"
#include "alimer.h"
#if defined(ALIMER_GPU)
#include "alimer_gpu.h"
#endif

#include <SDL3/SDL.h>

#if defined(SDL_PLATFORM_MACOS)
#include <SDL3/SDL_metal.h>
#endif

#include <deque>

namespace
{
    static void AlimerLog_SDL(void* /*userdata*/, int /*category*/, SDL_LogPriority priority, const char* message)
    {
        switch (priority)
        {
            case SDL_LOG_PRIORITY_VERBOSE:
                alimerLog(LogCategory_Platform, LogLevel_Trace, message);
                break;
            case SDL_LOG_PRIORITY_DEBUG:
                alimerLog(LogCategory_Platform, LogLevel_Debug, message);
                break;
            case SDL_LOG_PRIORITY_INFO:
                alimerLog(LogCategory_Platform, LogLevel_Info, message);
                break;
            case SDL_LOG_PRIORITY_WARN:
                alimerLog(LogCategory_Platform, LogLevel_Warn, message);
                break;
            case SDL_LOG_PRIORITY_ERROR:
                alimerLog(LogCategory_Platform, LogLevel_Error, message);
                break;
            case SDL_LOG_PRIORITY_CRITICAL:
                alimerLog(LogCategory_Platform, LogLevel_Fatal, message);
                break;
            default:
                break;
        }
    }

    constexpr KeyboardKey FromSDLKeyboardKey(SDL_Scancode code)
    {
        switch (code)
        {
            case SDL_SCANCODE_KP_BACKSPACE: return KeyboardKey_Backspace;
            case SDL_SCANCODE_KP_TAB:       return KeyboardKey_Tab;
            case SDL_SCANCODE_KP_CLEAR:     return KeyboardKey_Clear;
            case SDL_SCANCODE_RETURN:       return KeyboardKey_Return;
            case SDL_SCANCODE_PAUSE:        return KeyboardKey_Pause;
            case SDL_SCANCODE_CAPSLOCK:     return KeyboardKey_CapsLock;
            case SDL_SCANCODE_LANG3:        return KeyboardKey_Kana;
                //ImeOn = 0x16,
                //Kanji = 0x19,
                //ImeOff = 0x1a,
            case SDL_SCANCODE_ESCAPE:       return KeyboardKey_Escape;
                //ImeConvert = 0x1c,
                //ImeNoConvert = 0x1d,

            case SDL_SCANCODE_SPACE:        return KeyboardKey_Space;
            case SDL_SCANCODE_PAGEUP:       return KeyboardKey_PageUp;
            case SDL_SCANCODE_PAGEDOWN:     return KeyboardKey_PageDown;
            case SDL_SCANCODE_END:          return KeyboardKey_End;
            case SDL_SCANCODE_HOME:         return KeyboardKey_Home;
            case SDL_SCANCODE_LEFT:         return KeyboardKey_Left;
            case SDL_SCANCODE_UP:           return KeyboardKey_Up;
            case SDL_SCANCODE_RIGHT:        return KeyboardKey_Right;
            case SDL_SCANCODE_DOWN:         return KeyboardKey_Down;
            case SDL_SCANCODE_SELECT:       return KeyboardKey_Select;
                //case SDL_SCANCODE_PRINTSCREEN:  return KeyboardKeys::Print;
            case SDL_SCANCODE_EXECUTE:      return KeyboardKey_Execute;
            case SDL_SCANCODE_PRINTSCREEN:  return KeyboardKey_PrintScreen;
            case SDL_SCANCODE_INSERT:       return KeyboardKey_Insert;
            case SDL_SCANCODE_DELETE:       return KeyboardKey_Delete;
            case SDL_SCANCODE_HELP:         return KeyboardKey_Help;

            case SDL_SCANCODE_1:            return KeyboardKey_D1;
            case SDL_SCANCODE_2:            return KeyboardKey_D2;
            case SDL_SCANCODE_3:            return KeyboardKey_D3;
            case SDL_SCANCODE_4:            return KeyboardKey_D4;
            case SDL_SCANCODE_5:            return KeyboardKey_D5;
            case SDL_SCANCODE_6:            return KeyboardKey_D6;
            case SDL_SCANCODE_7:            return KeyboardKey_D7;
            case SDL_SCANCODE_8:            return KeyboardKey_D8;
            case SDL_SCANCODE_9:            return KeyboardKey_D9;
            case SDL_SCANCODE_0:            return KeyboardKey_D0;

            case SDL_SCANCODE_A:            return KeyboardKey_A;
            case SDL_SCANCODE_B:            return KeyboardKey_B;
            case SDL_SCANCODE_C:            return KeyboardKey_C;
            case SDL_SCANCODE_D:            return KeyboardKey_D;
            case SDL_SCANCODE_E:            return KeyboardKey_E;
            case SDL_SCANCODE_F:            return KeyboardKey_F;
            case SDL_SCANCODE_G:            return KeyboardKey_G;
            case SDL_SCANCODE_H:            return KeyboardKey_H;
            case SDL_SCANCODE_I:            return KeyboardKey_I;
            case SDL_SCANCODE_J:            return KeyboardKey_J;
            case SDL_SCANCODE_K:            return KeyboardKey_K;
            case SDL_SCANCODE_L:            return KeyboardKey_L;
            case SDL_SCANCODE_M:            return KeyboardKey_M;
            case SDL_SCANCODE_N:            return KeyboardKey_N;
            case SDL_SCANCODE_O:            return KeyboardKey_O;
            case SDL_SCANCODE_P:            return KeyboardKey_P;
            case SDL_SCANCODE_Q:            return KeyboardKey_Q;
            case SDL_SCANCODE_R:            return KeyboardKey_R;
            case SDL_SCANCODE_S:            return KeyboardKey_S;
            case SDL_SCANCODE_T:            return KeyboardKey_T;
            case SDL_SCANCODE_U:            return KeyboardKey_U;
            case SDL_SCANCODE_V:            return KeyboardKey_V;
            case SDL_SCANCODE_W:            return KeyboardKey_W;
            case SDL_SCANCODE_X:            return KeyboardKey_X;
            case SDL_SCANCODE_Y:            return KeyboardKey_Y;
            case SDL_SCANCODE_Z:            return KeyboardKey_Z;

            case SDL_SCANCODE_LGUI:         return KeyboardKey_LeftSuper;
            case SDL_SCANCODE_RGUI:         return KeyboardKey_RightSuper;
            case SDL_SCANCODE_APPLICATION:  return KeyboardKey_Apps;
            case SDL_SCANCODE_SLEEP:        return KeyboardKey_Sleep;

            case SDL_SCANCODE_KP_0:         return KeyboardKey_Numpad0;
            case SDL_SCANCODE_KP_1:         return KeyboardKey_Numpad1;
            case SDL_SCANCODE_KP_2:         return KeyboardKey_Numpad2;
            case SDL_SCANCODE_KP_3:         return KeyboardKey_Numpad3;
            case SDL_SCANCODE_KP_4:         return KeyboardKey_Numpad4;
            case SDL_SCANCODE_KP_5:         return KeyboardKey_Numpad5;
            case SDL_SCANCODE_KP_6:         return KeyboardKey_Numpad6;
            case SDL_SCANCODE_KP_7:         return KeyboardKey_Numpad7;
            case SDL_SCANCODE_KP_8:         return KeyboardKey_Numpad8;
            case SDL_SCANCODE_KP_9:         return KeyboardKey_Numpad9;
            case SDL_SCANCODE_KP_MULTIPLY:  return KeyboardKey_Multiply;
            case SDL_SCANCODE_KP_PLUS:      return KeyboardKey_Add;
            case SDL_SCANCODE_SEPARATOR:    return KeyboardKey_Separator;
            case SDL_SCANCODE_KP_MINUS:     return KeyboardKey_Subtract;
            case SDL_SCANCODE_KP_PERIOD:    return KeyboardKey_Decimal;
            case SDL_SCANCODE_KP_DIVIDE:    return KeyboardKey_Divide;
                //case SDL_SCANCODE_KP_ENTER:   return Key::Divide;

            case SDL_SCANCODE_F1:           return KeyboardKey_F1;
            case SDL_SCANCODE_F2:           return KeyboardKey_F2;
            case SDL_SCANCODE_F3:           return KeyboardKey_F3;
            case SDL_SCANCODE_F4:           return KeyboardKey_F4;
            case SDL_SCANCODE_F5:           return KeyboardKey_F5;
            case SDL_SCANCODE_F6:           return KeyboardKey_F6;
            case SDL_SCANCODE_F7:           return KeyboardKey_F7;
            case SDL_SCANCODE_F8:           return KeyboardKey_F8;
            case SDL_SCANCODE_F9:           return KeyboardKey_F9;
            case SDL_SCANCODE_F10:          return KeyboardKey_F10;
            case SDL_SCANCODE_F11:          return KeyboardKey_F11;
            case SDL_SCANCODE_F12:          return KeyboardKey_F12;
            case SDL_SCANCODE_F13:          return KeyboardKey_F13;
            case SDL_SCANCODE_F14:          return KeyboardKey_F14;
            case SDL_SCANCODE_F15:          return KeyboardKey_F15;
            case SDL_SCANCODE_F16:          return KeyboardKey_F16;
            case SDL_SCANCODE_F17:          return KeyboardKey_F17;
            case SDL_SCANCODE_F18:          return KeyboardKey_F18;
            case SDL_SCANCODE_F19:          return KeyboardKey_F19;
            case SDL_SCANCODE_F20:          return KeyboardKey_F20;
            case SDL_SCANCODE_F21:          return KeyboardKey_F21;
            case SDL_SCANCODE_F22:          return KeyboardKey_F22;
            case SDL_SCANCODE_F23:          return KeyboardKey_F23;
            case SDL_SCANCODE_F24:          return KeyboardKey_F24;

            case SDL_SCANCODE_NUMLOCKCLEAR: return KeyboardKey_NumLock;
            case SDL_SCANCODE_SCROLLLOCK:   return KeyboardKey_ScrollLock;

            case SDL_SCANCODE_LSHIFT:       return KeyboardKey_LeftShift;
            case SDL_SCANCODE_RSHIFT:       return KeyboardKey_RightShift;
            case SDL_SCANCODE_LCTRL:        return KeyboardKey_LeftControl;
            case SDL_SCANCODE_RCTRL:        return KeyboardKey_RightControl;
            case SDL_SCANCODE_LALT:         return KeyboardKey_LeftAlt;
            case SDL_SCANCODE_RALT:         return KeyboardKey_RightAlt;

            default:
                return KeyboardKey_None;
        }
    }

    /*constexpr KeyModifiers FromSDLKeyModifiers(Uint16 mod)
    {
        KeyModifiers result = KeyModifiers::None;
        if ((mod & SDL_KMOD_ALT) != 0)
            result |= KeyModifiers::Alt;
        if ((mod & SDL_KMOD_CTRL) != 0)
            result |= KeyModifiers::Control;
        if ((mod & SDL_KMOD_SHIFT) != 0)
            result |= KeyModifiers::Shift;
        if ((mod & SDL_KMOD_GUI) != 0)
            result |= KeyModifiers::Super;

        return result;
    }*/

    constexpr MouseButton FromSDLMouseButton(Uint8 id)
    {
        switch (id)
        {
            case SDL_BUTTON_LEFT:       return MouseButton_Left;
            case SDL_BUTTON_RIGHT:      return MouseButton_Right;
            case SDL_BUTTON_MIDDLE:     return MouseButton_Middle;
            case SDL_BUTTON_X1:         return MouseButton_X1;
            case SDL_BUTTON_X2:         return MouseButton_X2;
            default:                    return MouseButton_Left;
        }
    }

    constexpr WindowEventType ToWindowEventType(uint32_t id)
    {
        switch (id)
        {
            case SDL_EVENT_WINDOW_SHOWN:                return WindowEventType_Shown;
            case SDL_EVENT_WINDOW_HIDDEN:               return WindowEventType_Hidden;
            case SDL_EVENT_WINDOW_EXPOSED:              return WindowEventType_Exposed;
            case SDL_EVENT_WINDOW_MOVED:                return WindowEventType_Moved;
            case SDL_EVENT_WINDOW_RESIZED:              return WindowEventType_Resized;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:   return WindowEventType_SizeChanged;
            case SDL_EVENT_WINDOW_MAXIMIZED:            return WindowEventType_Maximized;
            case SDL_EVENT_WINDOW_MINIMIZED:            return WindowEventType_Minimized;
            case SDL_EVENT_WINDOW_RESTORED:             return WindowEventType_Restored;
            case SDL_EVENT_WINDOW_MOUSE_ENTER:          return WindowEventType_Enter;
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:          return WindowEventType_Leave;
            case SDL_EVENT_WINDOW_FOCUS_GAINED:         return WindowEventType_FocusGained;
            case SDL_EVENT_WINDOW_FOCUS_LOST:           return WindowEventType_FocusLost;
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:      return WindowEventType_CloseRequested;
            default:
                return WindowEventType_None;
        }
    }

    inline PlatformEvent ToEvent(const SDL_Event& e)
    {
        PlatformEvent ev{};
        switch (e.type)
        {
            case SDL_EVENT_QUIT:
                ev.type = EventType_Quit;
                break;
            case SDL_EVENT_TERMINATING:
                ev.type = EventType_Terminating;
                break;
            case SDL_EVENT_LOW_MEMORY:
                ev.type = EventType_LowMemory;
                break;
            case SDL_EVENT_WILL_ENTER_BACKGROUND:
                ev.type = EventType_WillEnterBackground;
                break;
            case SDL_EVENT_DID_ENTER_BACKGROUND:
                ev.type = EventType_DidEnterBackground;
                break;
            case SDL_EVENT_WILL_ENTER_FOREGROUND:
                ev.type = EventType_WillEnterForeground;
                break;
            case SDL_EVENT_DID_ENTER_FOREGROUND:
                ev.type = EventType_DidEnterForeground;
                break;
            case SDL_EVENT_LOCALE_CHANGED:
                ev.type = EventType_LocaleChanged;
                break;
            case SDL_EVENT_SYSTEM_THEME_CHANGED:
                ev.type = EventType_SystemThemeChanged;
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                ev.type = (e.type == SDL_EVENT_KEY_DOWN) ? EventType_KeyDown : EventType_KeyUp;
                ev.key.windowID = e.key.windowID;
                ev.key.key = FromSDLKeyboardKey(e.key.scancode);
                ev.key.alt = (e.key.mod & SDL_KMOD_ALT) != 0;
                ev.key.ctrl = (e.key.mod & SDL_KMOD_CTRL) != 0;
                ev.key.shift = (e.key.mod & SDL_KMOD_SHIFT) != 0;
                ev.key.system = (e.key.mod & SDL_KMOD_GUI) != 0;
                break;
            case SDL_EVENT_TEXT_INPUT:
                ev.type = EventType_TextInput;
                ev.text.windowID = e.text.windowID;
                ev.text.text = e.text.text;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                ev.type = EventType_MouseMotion;
                ev.motion.windowID = e.motion.windowID;

                ev.motion.x = e.motion.x;
                ev.motion.y = e.motion.y;
                ev.motion.xRelative = e.motion.xrel;
                ev.motion.yRelative = e.motion.yrel;

                // SDL_GetRelativeMouseMode?

                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                ev.type = (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? EventType_MouseButtonDown : EventType_MouseButtonUp;
                ev.button.windowID = e.button.windowID;
                ev.button.button = FromSDLMouseButton(e.button.button);
                ev.button.x = e.button.x;
                ev.button.y = e.button.y;
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                ev.type = EventType_MouseWheel;
                ev.wheel.windowID = e.wheel.windowID;
                //ev.wheel.which = e.wheel.which;
                ev.wheel.x = e.wheel.x;
                ev.wheel.y = e.wheel.y;
                break;
            case SDL_EVENT_MOUSE_ADDED:
                ev.type = EventType_MouseAdded;
                break;
            case SDL_EVENT_MOUSE_REMOVED:
                ev.type = EventType_MouseAdded;
                break;
            case SDL_EVENT_CLIPBOARD_UPDATE:
                ev.type = EventType_ClipboardUpdate;
                break;
            default:
                if (e.type >= SDL_EVENT_WINDOW_FIRST && e.type <= SDL_EVENT_WINDOW_LAST)
                {
                    ev.type = EventType_Window;
                    ev.window.windowID = e.window.windowID;
                    ev.window.type = ToWindowEventType(e.type);
                    ev.window.data1 = e.window.data1;
                    ev.window.data2 = e.window.data2;
                }
                else
                {
                    ev.type = EventType_Unknown;
                }
        }

        return ev;
    }
}

struct Window
{
    SDL_Window* handle = nullptr;
    SDL_WindowID id = 0;
};

static struct {
    bool initialized;
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

    //SDL_SetHint(SDL_HINT_APP_NAME, game.Name);
    //SDL_SetHint(SDL_HINT_WINDOWS_CLOSE_ON_ALT_F4, "0");
    //SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    //SDL_SetHint(SDL_HINT_APP_NAME, settings.name.c_str());

#if defined(_DEBUG)
    SDL_SetLogPriority(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_DEBUG);
#endif
    SDL_SetLogOutputFunction(AlimerLog_SDL, nullptr);
    // Init SDL
    const Uint32 sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_GAMEPAD;
    if (!SDL_Init(sdl_init_flags))
    {
        alimerLogError(LogCategory_Platform, "Alimer: SDL_Init Failed: %s", SDL_GetError());
        return false;
    }

    const int version = SDL_GetVersion();
    alimerLogInfo(LogCategory_Platform, "SDL Initialized: v%d.%d.%d, revision: %s",
        SDL_VERSIONNUM_MAJOR(version),
        SDL_VERSIONNUM_MINOR(version),
        SDL_VERSIONNUM_MICRO(version),
        SDL_GetRevision()
    );

    state.initialized = true;
    return true;
}

void alimerPlatformShutdown(void)
{
    if (!state.initialized)
        return;

    SDL_Quit();
    memset(&state, 0, sizeof(state));
}

bool alimerPlatformPollEvent(PlatformEvent* evt)
{
    ALIMER_ASSERT(state.initialized);

    SDL_Event ev{};
    while (SDL_PollEvent(&ev) != 0)
    {
        auto e = ToEvent(ev);
        PushEvent(std::move(e));
    }

    return PopEvent(evt);
}

Window* alimerWindowCreate(const WindowDesc* desc)
{
    ALIMER_ASSERT(desc);

    const bool fullscreen = desc->flags & WindowFlags_Fullscreen;

    SDL_WindowFlags windowFlags = SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN;

    if (fullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }
    else
    {
        if (desc->flags & WindowFlags_Hidden)
            windowFlags |= SDL_WINDOW_HIDDEN;

        if (desc->flags & WindowFlags_Borderless)
            windowFlags |= SDL_WINDOW_BORDERLESS;

        if (desc->flags & WindowFlags_Resizable)
            windowFlags |= SDL_WINDOW_RESIZABLE;

        if (desc->flags & WindowFlags_Maximized)
            windowFlags |= SDL_WINDOW_MAXIMIZED;

        if (desc->flags & WindowFlags_AlwaysOnTop)
            windowFlags |= SDL_WINDOW_ALWAYS_ON_TOP;
    }

    SDL_Window* handle = SDL_CreateWindow(desc->title,
        static_cast<int>(desc->width),
        static_cast<int>(desc->height),
        windowFlags);
    if (handle == nullptr)
    {
        alimerLogError(LogCategory_Platform, "Alimer: SDL_CreateWindow Failed: %s", SDL_GetError());
        return nullptr;
    }

    if (desc->icon.data)
    {
        SDL_Surface* surface = SDL_CreateSurfaceFrom(
            static_cast<int>(desc->icon.width),
            static_cast<int>(desc->icon.height),
            SDL_PIXELFORMAT_RGBA8888,
            (void*)desc->icon.data,
            static_cast<int>(desc->icon.width * 4));
        if (!surface)
        {
            alimerLogError(LogCategory_Platform, "Alimer: SDL_CreateSurfaceFrom Failed: %s", SDL_GetError());
            SDL_DestroyWindow(handle);
            return nullptr;
        }

        SDL_SetWindowIcon(handle, surface);
        SDL_DestroySurface(surface);
    }

    Window* window = new Window();
    window->handle = handle;
    window->id = SDL_GetWindowID(handle);
    return window;
}

void alimerWindowDestroy(Window* window)
{
    if (window->handle != nullptr)
    {
        SDL_DestroyWindow(window->handle);
        window->handle = nullptr;
    }

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

    return window->handle != nullptr;
}

void alimerWindowSetPosition(Window* window, int32_t x, int32_t y)
{
    ALIMER_ASSERT(window != nullptr);

    SDL_SetWindowPosition(window->handle, x, y);
}

void alimerWindowGetPosition(Window* window, int32_t* x, int32_t* y)
{
    ALIMER_ASSERT(window != nullptr);

    SDL_GetWindowPosition(window->handle, x, y);
}

void alimerWindowSetCentered(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    SDL_SetWindowPosition(window->handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void alimerWindowSetSize(Window* window, uint32_t width, uint32_t height)
{
    ALIMER_ASSERT(window != nullptr);

    SDL_SetWindowSize(window->handle, static_cast<int>(width), static_cast<int>(height));
}

void alimerWindowGetSize(Window* window, uint32_t* width, uint32_t* height)
{
    ALIMER_ASSERT(window != nullptr);

    int w = 0, h = 0;
    SDL_GetWindowSize(window->handle, &w, &h);
    if (width)
        *width = static_cast<uint32_t>(w);
    if (height)
        *height = static_cast<uint32_t>(h);
}

void alimerWindowGetSizeInPixels(Window* window, uint32_t* width, uint32_t* height)
{
    ALIMER_ASSERT(window != nullptr);

    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window->handle, &w, &h);
    if (width)
        *width = static_cast<uint32_t>(w);
    if (height)
        *height = static_cast<uint32_t>(h);
}

void alimerWindowSetTitle(Window* window, const char* title)
{
    ALIMER_ASSERT(window != nullptr);

    SDL_SetWindowTitle(window->handle, title);
}

const char* alimerWindowGetTitle(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    return SDL_GetWindowTitle(window->handle);
}

bool alimerWindowIsMinimized(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    const SDL_WindowFlags flags = SDL_GetWindowFlags(window->handle);
    return (flags & SDL_WINDOW_MINIMIZED) != 0;
}

bool alimerWindowIsMaximized(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    const SDL_WindowFlags flags = SDL_GetWindowFlags(window->handle);
    return (flags & SDL_WINDOW_MAXIMIZED) != 0;
}

bool alimerWindowIsFullscreen(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    const SDL_WindowFlags flags = SDL_GetWindowFlags(window->handle);
    return (flags & SDL_WINDOW_FULLSCREEN) != 0;
}

void alimerWindowSetFullscreen(Window* window, bool value)
{
    ALIMER_ASSERT(window != nullptr);

    SDL_SetWindowFullscreen(window->handle, value);
}

bool alimerWindowHasFocus(Window* window)
{
    ALIMER_ASSERT(window != nullptr);
    const SDL_WindowFlags flags = SDL_GetWindowFlags(window->handle);
    return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
}

void alimerWindowShow(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    SDL_ShowWindow(window->handle);
}

void alimerWindowHide(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    SDL_HideWindow(window->handle);
}

void alimerWindowMaximize(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    SDL_MaximizeWindow(window->handle);
}

void alimerWindowMinimize(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    SDL_MinimizeWindow(window->handle);
}

void alimerWindowRestore(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    SDL_RestoreWindow(window->handle);
}

void alimerWindowFocus(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

    SDL_RaiseWindow(window->handle);
}

#if defined(ALIMER_GPU)
GPUSurfaceHandle* alimerWindowCreateSurfaceHandle(Window* window)
{
    SDL_PropertiesID props = SDL_GetWindowProperties(window->handle);

#if defined(SDL_PLATFORM_WIN32)
    return agpuSurfaceHandleCreateFromWin32(
        SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr)
    );
#elif defined(SDL_PLATFORM_MACOS)
    //NSWindow* nswindow = (__bridge NSWindow*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
#elif defined(SDL_PLATFORM_LINUX)
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0)
    {
    }
    else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0)
    {

    }
    return nullptr;
#elif defined(SDL_PLATFORM_IOS)
    //UIWindow* uiwindow = (__bridge UIWindow*)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
#else
    return nullptr;
#endif
}
#endif

void* alimerWindowGetNativeHandle(Window* window)
{
    ALIMER_ASSERT(window != nullptr);

#if defined(SDL_PLATFORM_WIN32)
    return SDL_GetPointerProperty(SDL_GetWindowProperties(window->handle), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#elif defined(SDL_PLATFORM_MACOS) && defined(TODO)
    NSWindow* ns_window = (__bridge NSWindow*)SDL_GetPointerProperty(SDL_GetWindowProperties(window->handle), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
    //[ns_window.contentView setWantsLayer : YES] ;
    //id metal_layer = [CAMetalLayer layer];
    //[ns_window.contentView setLayer : metal_layer] ;
    return ns_window;
#elif defined(SDL_PLATFORM_LINUX) && defined(TODO)
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0)
    {
        Display* xdisplay = (Display*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        Window xwindow = (Window)SDL_GetNumberProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        if (xdisplay && xwindow)
        {
        }
    }
    else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0)
    {
        struct wl_display* display = (struct wl_display*)SDL_GetPointerProperty(SDL_GetWindowProperties(window->handle), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
        struct wl_surface* surface = (struct wl_surface*)SDL_GetPointerProperty(SDL_GetWindowProperties(window->handle), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
        if (display && surface)
        {
            surface = RHISurface::CreateWayland(display, surface);
        }
    }
#else
    return nullptr;
#endif
}

/* Clipboard */
bool alimerHasClipboardText(void)
{
    return SDL_HasClipboardText();
}

const char* alimerClipboardGetText(void)
{
    return SDL_GetClipboardText();
}

void alimerClipboardSetText(const char* text)
{
    SDL_SetClipboardText(text);
}
