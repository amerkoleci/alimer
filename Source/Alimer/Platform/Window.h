// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Object.h"
#include "Alimer/Core/Signal.h"
#include "Alimer/Platform/Types.h"

namespace Alimer
{
    class WindowImpl;

    /// Class that defines an OS window.
    class ALIMER_API Window final 
    {
        friend class Application;

    public:
        Window(const std::string& title, uint32_t width, uint32_t height, WindowFlags flags = WindowFlags::Resizable | WindowFlags::Hidden);

        /// Destructor.
        ~Window();

        // Non-copyable and non-movable
        ALIMER_DISABLE_COPY_MOVE(Window);

        [[nodiscard]] uint32_t GetId() const { return id; }

#if TODO
        [[nodiscard]] Int2 GetPosition() const;
        void SetPosition(int32_t x, int32_t y) noexcept;
        void SetPosition(const Int2& position) noexcept;

        [[nodiscard]] SizeI GetSize() const;
        [[nodiscard]] SizeI GetDrawableSize() const;
        [[nodiscard]] float GetAspectRatio() const;
        void SetSize(int32_t width, int32_t height) noexcept;
        void SetSize(const SizeI& size) noexcept;
#endif // TODO


        [[nodiscard]] const std::string& GetTitle() const { return _title; }
        void SetTitle(std::string_view title);

        void Show();
        void Hide();
        void Maximize();
        void Minimize();
        void Restore();

        [[nodiscard]] bool IsOpen() const;
        [[nodiscard]] bool IsMinimized() const;
        [[nodiscard]] bool IsFullscreen() const;
        [[nodiscard]] bool IsFocused() const;
        [[nodiscard]] bool IsCursorVisible() const;

        void RequestFocus();
        void SetFullscreen(bool value);
        void SetCursorVisible(bool value);

    private:
        std::unique_ptr<WindowImpl> impl;
        uint32_t id{};
        std::string _title;
    };
}
