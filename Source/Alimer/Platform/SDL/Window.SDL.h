// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Platform/Window.h"

struct SDL_Window;

namespace Alimer
{
    class WindowSDL final : public Window
    {
    public:
        WindowSDL(const std::string& title, uint32_t width, uint32_t height, WindowFlags flags);
        ~WindowSDL() override;

        void SetTitle(std::string_view title)  override;

        void Show() override;
        void Hide() override;
        void Maximize() override;
        void Minimize() override;
        void Restore() override;

        bool IsOpen() const override;
        bool IsMinimized() const override;
        bool IsFullscreen() const override;
        bool IsFocused() const override;
        bool IsCursorVisible() const;

        void RequestFocus() override;
        void SetFullscreen(bool value) override;
        void SetCursorVisible(bool value) override;

        /// <summary>
        /// Called by SDL resize event
        /// </summary>
        void OnResized();

        [[nodiscard]] SDL_Window* GetHandle() const noexcept { return _handle; }

    private:
        SDL_Window* _handle;
        bool _fullscreen;
    };
}
