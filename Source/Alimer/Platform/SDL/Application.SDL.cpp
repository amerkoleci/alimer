// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Application.h"
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
                    //Input::OnKeyboardKey(
                    //    FromSDLKeyboardKey(event.key.scancode),
                    //    FromSDLKeyModifiers(event.key.mod),
                    //    event.type == SDL_EVENT_KEY_DOWN
                    //);

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
                    //Input::OnMouseButton(FromSDL(event.button.button), event.button.x, event.button.y, event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                {
                    //float deltaX = event.motion.xrel;
                    //float deltaY = event.motion.yrel;
                    //Input::OnMouseMove(event.motion.x, event.motion.y);
                    break;
                }
                case SDL_EVENT_MOUSE_WHEEL:
                    //Input::OnMouseWheel(event.wheel.x, event.wheel.y);
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
