// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Application.h"
#include "Alimer/Input.h"
#include "Alimer/Platform/SDL/Window.SDL.h"
#include "Alimer/Core/Log.h"
#include <SDL3/SDL.h>

using namespace Alimer;

namespace
{
    static void AlimerLog_SDL(void* /*userdata*/, int /*category*/, SDL_LogPriority priority, const char* message)
    {
        switch (priority)
        {
            case SDL_LOG_PRIORITY_VERBOSE:
                Log::Trace(message);
                break;
            case SDL_LOG_PRIORITY_DEBUG:
                Log::Debug(message);
                break;
            case SDL_LOG_PRIORITY_INFO:
                Log::Info(message);
                break;
            case SDL_LOG_PRIORITY_WARN:
                Log::Warn(message);
                break;
            case SDL_LOG_PRIORITY_ERROR:
                Log::Error(message);
                break;
            case SDL_LOG_PRIORITY_CRITICAL:
                Log::Critical(message);
                break;
            default:
                break;
        }
    }


    constexpr KeyboardKeys FromSDLKeyboardKey(SDL_Scancode code)
    {
        switch (code)
        {
            case SDL_SCANCODE_KP_BACKSPACE: return KeyboardKeys::Backspace;
            case SDL_SCANCODE_KP_TAB:       return KeyboardKeys::Tab;
            case SDL_SCANCODE_KP_CLEAR:     return KeyboardKeys::Clear;
            case SDL_SCANCODE_RETURN:       return KeyboardKeys::Return;
            case SDL_SCANCODE_PAUSE:        return KeyboardKeys::Pause;
            case SDL_SCANCODE_CAPSLOCK:     return KeyboardKeys::CapsLock;
            case SDL_SCANCODE_LANG3:        return KeyboardKeys::Kana;
                //ImeOn = 0x16,
                //Kanji = 0x19,
                //ImeOff = 0x1a,
            case SDL_SCANCODE_ESCAPE:       return KeyboardKeys::Escape;
                //ImeConvert = 0x1c,
                //ImeNoConvert = 0x1d,

            case SDL_SCANCODE_SPACE:        return KeyboardKeys::Space;
            case SDL_SCANCODE_PAGEUP:       return KeyboardKeys::PageUp;
            case SDL_SCANCODE_PAGEDOWN:     return KeyboardKeys::PageDown;
            case SDL_SCANCODE_END:          return KeyboardKeys::End;
            case SDL_SCANCODE_HOME:         return KeyboardKeys::Home;
            case SDL_SCANCODE_LEFT:         return KeyboardKeys::Left;
            case SDL_SCANCODE_UP:           return KeyboardKeys::Up;
            case SDL_SCANCODE_RIGHT:        return KeyboardKeys::Right;
            case SDL_SCANCODE_DOWN:         return KeyboardKeys::Down;
            case SDL_SCANCODE_SELECT:       return KeyboardKeys::Select;
                //case SDL_SCANCODE_PRINTSCREEN:  return KeyboardKeys::Print;
            case SDL_SCANCODE_EXECUTE:      return KeyboardKeys::Execute;
            case SDL_SCANCODE_PRINTSCREEN:  return KeyboardKeys::PrintScreen;
            case SDL_SCANCODE_INSERT:       return KeyboardKeys::Insert;
            case SDL_SCANCODE_DELETE:       return KeyboardKeys::Delete;
            case SDL_SCANCODE_HELP:         return KeyboardKeys::Help;

            case SDL_SCANCODE_1:            return KeyboardKeys::D1;
            case SDL_SCANCODE_2:            return KeyboardKeys::D2;
            case SDL_SCANCODE_3:            return KeyboardKeys::D3;
            case SDL_SCANCODE_4:            return KeyboardKeys::D4;
            case SDL_SCANCODE_5:            return KeyboardKeys::D5;
            case SDL_SCANCODE_6:            return KeyboardKeys::D6;
            case SDL_SCANCODE_7:            return KeyboardKeys::D7;
            case SDL_SCANCODE_8:            return KeyboardKeys::D8;
            case SDL_SCANCODE_9:            return KeyboardKeys::D9;
            case SDL_SCANCODE_0:            return KeyboardKeys::D0;

            case SDL_SCANCODE_A:            return KeyboardKeys::A;
            case SDL_SCANCODE_B:            return KeyboardKeys::B;
            case SDL_SCANCODE_C:            return KeyboardKeys::C;
            case SDL_SCANCODE_D:            return KeyboardKeys::D;
            case SDL_SCANCODE_E:            return KeyboardKeys::E;
            case SDL_SCANCODE_F:            return KeyboardKeys::F;
            case SDL_SCANCODE_G:            return KeyboardKeys::G;
            case SDL_SCANCODE_H:            return KeyboardKeys::H;
            case SDL_SCANCODE_I:            return KeyboardKeys::I;
            case SDL_SCANCODE_J:            return KeyboardKeys::J;
            case SDL_SCANCODE_K:            return KeyboardKeys::K;
            case SDL_SCANCODE_L:            return KeyboardKeys::L;
            case SDL_SCANCODE_M:            return KeyboardKeys::M;
            case SDL_SCANCODE_N:            return KeyboardKeys::N;
            case SDL_SCANCODE_O:            return KeyboardKeys::O;
            case SDL_SCANCODE_P:            return KeyboardKeys::P;
            case SDL_SCANCODE_Q:            return KeyboardKeys::Q;
            case SDL_SCANCODE_R:            return KeyboardKeys::R;
            case SDL_SCANCODE_S:            return KeyboardKeys::S;
            case SDL_SCANCODE_T:            return KeyboardKeys::T;
            case SDL_SCANCODE_U:            return KeyboardKeys::U;
            case SDL_SCANCODE_V:            return KeyboardKeys::V;
            case SDL_SCANCODE_W:            return KeyboardKeys::W;
            case SDL_SCANCODE_X:            return KeyboardKeys::X;
            case SDL_SCANCODE_Y:            return KeyboardKeys::Y;
            case SDL_SCANCODE_Z:            return KeyboardKeys::Z;

            case SDL_SCANCODE_LGUI:         return KeyboardKeys::LeftSuper;
            case SDL_SCANCODE_RGUI:         return KeyboardKeys::RightSuper;
            case SDL_SCANCODE_APPLICATION:  return KeyboardKeys::Apps;
            case SDL_SCANCODE_SLEEP:        return KeyboardKeys::Sleep;

            case SDL_SCANCODE_KP_0:         return KeyboardKeys::Numpad0;
            case SDL_SCANCODE_KP_1:         return KeyboardKeys::Numpad1;
            case SDL_SCANCODE_KP_2:         return KeyboardKeys::Numpad2;
            case SDL_SCANCODE_KP_3:         return KeyboardKeys::Numpad3;
            case SDL_SCANCODE_KP_4:         return KeyboardKeys::Numpad4;
            case SDL_SCANCODE_KP_5:         return KeyboardKeys::Numpad5;
            case SDL_SCANCODE_KP_6:         return KeyboardKeys::Numpad6;
            case SDL_SCANCODE_KP_7:         return KeyboardKeys::Numpad7;
            case SDL_SCANCODE_KP_8:         return KeyboardKeys::Numpad8;
            case SDL_SCANCODE_KP_9:         return KeyboardKeys::Numpad9;
            case SDL_SCANCODE_KP_MULTIPLY:  return KeyboardKeys::Multiply;
            case SDL_SCANCODE_KP_PLUS:      return KeyboardKeys::Add;
            case SDL_SCANCODE_SEPARATOR:    return KeyboardKeys::Separator;
            case SDL_SCANCODE_KP_MINUS:     return KeyboardKeys::Subtract;
            case SDL_SCANCODE_KP_PERIOD:    return KeyboardKeys::Decimal;
            case SDL_SCANCODE_KP_DIVIDE:    return KeyboardKeys::Divide;
                //case SDL_SCANCODE_KP_ENTER:   return Key::Divide;

            case SDL_SCANCODE_F1:           return KeyboardKeys::F1;
            case SDL_SCANCODE_F2:           return KeyboardKeys::F2;
            case SDL_SCANCODE_F3:           return KeyboardKeys::F3;
            case SDL_SCANCODE_F4:           return KeyboardKeys::F4;
            case SDL_SCANCODE_F5:           return KeyboardKeys::F5;
            case SDL_SCANCODE_F6:           return KeyboardKeys::F6;
            case SDL_SCANCODE_F7:           return KeyboardKeys::F7;
            case SDL_SCANCODE_F8:           return KeyboardKeys::F8;
            case SDL_SCANCODE_F9:           return KeyboardKeys::F9;
            case SDL_SCANCODE_F10:          return KeyboardKeys::F10;
            case SDL_SCANCODE_F11:          return KeyboardKeys::F11;
            case SDL_SCANCODE_F12:          return KeyboardKeys::F12;
            case SDL_SCANCODE_F13:          return KeyboardKeys::F13;
            case SDL_SCANCODE_F14:          return KeyboardKeys::F14;
            case SDL_SCANCODE_F15:          return KeyboardKeys::F15;
            case SDL_SCANCODE_F16:          return KeyboardKeys::F16;
            case SDL_SCANCODE_F17:          return KeyboardKeys::F17;
            case SDL_SCANCODE_F18:          return KeyboardKeys::F18;
            case SDL_SCANCODE_F19:          return KeyboardKeys::F19;
            case SDL_SCANCODE_F20:          return KeyboardKeys::F20;
            case SDL_SCANCODE_F21:          return KeyboardKeys::F21;
            case SDL_SCANCODE_F22:          return KeyboardKeys::F22;
            case SDL_SCANCODE_F23:          return KeyboardKeys::F23;
            case SDL_SCANCODE_F24:          return KeyboardKeys::F24;

            case SDL_SCANCODE_NUMLOCKCLEAR: return KeyboardKeys::NumLock;
            case SDL_SCANCODE_SCROLLLOCK:   return KeyboardKeys::ScrollLock;

            case SDL_SCANCODE_LSHIFT:       return KeyboardKeys::LeftShift;
            case SDL_SCANCODE_RSHIFT:       return KeyboardKeys::RightShift;
            case SDL_SCANCODE_LCTRL:        return KeyboardKeys::LeftControl;
            case SDL_SCANCODE_RCTRL:        return KeyboardKeys::RightControl;
            case SDL_SCANCODE_LALT:         return KeyboardKeys::LeftAlt;
            case SDL_SCANCODE_RALT:         return KeyboardKeys::RightAlt;

            default:
                return KeyboardKeys::None;
        }
    }

    constexpr KeyModifiers FromSDLKeyModifiers(Uint16 mod)
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
    }

    constexpr MouseButton FromSDL(Uint8 id)
    {
        switch (id)
        {
            case SDL_BUTTON_LEFT:
                return MouseButton::Left;
            case SDL_BUTTON_RIGHT:
                return MouseButton::Right;
            case SDL_BUTTON_MIDDLE:
                return MouseButton::Middle;
            case SDL_BUTTON_X1:
                return MouseButton::XButton1;
            case SDL_BUTTON_X2:
                return MouseButton::XButton2;
            default:
                return MouseButton::Left;
        }
    }
}


bool Application::PlatformInit()
{
#if defined(_DEBUG)
    SDL_SetLogPriority(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_DEBUG);
#endif
    SDL_SetLogOutputFunction(AlimerLog_SDL, nullptr);

    // Init SDL
    const Uint32 sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_GAMEPAD;
    if (!SDL_Init(sdl_init_flags))
    {
        LOGE("Alimer: SDL_Init Failed: {}", SDL_GetError());
        return false;
    }

    if (!_options.name.empty())
    {
        SDL_SetHint(SDL_HINT_APP_NAME, _options.name.c_str());
    }

    const int version = SDL_GetVersion();
    LOGI("SDL Initialized: v{}.{}.{}, revision: {}",
        SDL_VERSIONNUM_MAJOR(version),
        SDL_VERSIONNUM_MINOR(version),
        SDL_VERSIONNUM_MICRO(version),
        SDL_GetRevision()
    );

    return true;
}

void Application::PlatformShutdown()
{
    SDL_Quit();
}

void Application::PlatformRunMainLoop()
{
    const WindowDesc& windowDesc = _options.window;
    std::string title = windowDesc.title;
    if (title.empty())
        title = _options.name;

    _mainWindow = std::make_unique<Window>(title, windowDesc.width, windowDesc.height, windowDesc.flags);
    InitBeforeRun();

    _mainWindow->Show();
    _exitRequested = false;
    //ImGuiIO& io = ImGui::GetIO(); (void)io;
    while (!_exitRequested)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            //ImGui_ImplSDL3_ProcessEvent(&event);

            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    _exitRequested = true;
                    break;
                case SDL_EVENT_TERMINATING:
                    //ev.type = events::app_terminating;
                    break;
                case SDL_EVENT_LOW_MEMORY:
                    //ev.type = events::app_low_memory;
                    break;
                case SDL_EVENT_WILL_ENTER_BACKGROUND:
                    //ev.type = events::app_will_enter_background;
                    break;
                case SDL_EVENT_DID_ENTER_BACKGROUND:
                    //ev.type = events::app_did_enter_background;
                    break;
                case SDL_EVENT_WILL_ENTER_FOREGROUND:
                    //ev.type = events::app_will_enter_foreground;
                    break;
                case SDL_EVENT_DID_ENTER_FOREGROUND:
                    //ev.type = events::app_did_enter_foreground;
                    break;
                case SDL_EVENT_DISPLAY_ORIENTATION:
                    //ev.type = events::display_orientation;
                    break;
                case SDL_EVENT_DISPLAY_ADDED:
                    //ev.type = events::display_connected;
                    break;
                case SDL_EVENT_DISPLAY_REMOVED:
                    //ev.type = events::display_disconnected;
                    break;
                case SDL_EVENT_DISPLAY_MOVED:
                    //ev.type = events::display_moved;
                    break;
                case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
                    //ev.type = events::display_content_scale_changed;
                    break;
                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP:
                    Input::OnKeyboardKey(
                        FromSDLKeyboardKey(event.key.scancode),
                        FromSDLKeyModifiers(event.key.mod),
                        event.type == SDL_EVENT_KEY_DOWN
                    );

#if defined(_DEBUG)
                    if (event.key.scancode == SDL_SCANCODE_ESCAPE)
                    {
                        _exitRequested = true;
                    }
#endif
                    break;
                case SDL_EVENT_TEXT_INPUT:
                    //ev.type = events::text_input;
                    //ev.text.window_id = e.text.windowID;
                    //ev.text.text = e.text.text;
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    Input::OnMouseButton(
                        FromSDL(event.button.button),
                        event.button.x,
                        event.button.y,
                        event.type == SDL_EVENT_MOUSE_BUTTON_DOWN
                    );
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                {
                    //float deltaX = event.motion.xrel;
                    //float deltaY = event.motion.yrel;
                    Input::OnMouseMove(event.motion.x, event.motion.y);
                    break;
                }
                case SDL_EVENT_MOUSE_WHEEL:
                    Input::OnMouseWheel(event.wheel.x, event.wheel.y);
                    break;
                case SDL_EVENT_FINGER_DOWN:
                    //ev.type = events::finger_down;
                    break;
                case SDL_EVENT_FINGER_UP:
                    //ev.type = events::finger_up;
                    break;
                case SDL_EVENT_FINGER_MOTION:
                    //ev.type = events::finger_motion;
                    break;
                case SDL_EVENT_CLIPBOARD_UPDATE:
                    //ev.type = events::clipboard_update;
                    break;
                case SDL_EVENT_DROP_FILE:
                    //ev.type = events::drop_file;
                    //ev.drop.window_id = e.drop.windowID;
                    //if (e.drop.file != nullptr)
                    //{
                    //    ev.drop.file = e.drop.file;
                    //    SDL_free(e.drop.file);
                    //}
                    break;
                default:
                    if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST)
                    {
                        // Window events.
                        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                            event.window.windowID == _mainWindow->GetId())
                        {
                            _exitRequested = true;
                        }
                        else
                        {
                            if (event.type == SDL_EVENT_WINDOW_RESIZED)
                            {
                                //mainWindow->OnResized(event.window.data1, event.window.data2);
                                _mainWindow->OnResized();
                            }
                        }
                    }
                    break;
            }
        }

        if (_exitRequested)
        {
            break;
        }

        // Tick one frame
        Tick();

#if ALIMER_IMGUI
        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            //SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            //SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            //SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }
#endif // ALIMER_IMGUI

    }

    _mainWindow.reset();
}
