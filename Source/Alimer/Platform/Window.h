// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Object.h"
#include "Alimer/Core/Signal.h"
#include "Alimer/Platform/Types.h"

namespace Alimer
{
    /// Class that defines an OS window.
    class ALIMER_API Window 
    {
        friend class Application;

    public:
        /// Occurs when the client size is changed.
        Signal<> ClientSizeChanged;

        /// Destructor.
        virtual ~Window() = default;

        // Non-copyable and non-movable
        ALIMER_DISABLE_COPY_MOVE(Window);

        [[nodiscard]] uint32_t GetId() const { return _id; }

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
        virtual void SetTitle(std::string_view title) = 0;

        virtual void Show() = 0;
        virtual void Hide() = 0;
        virtual void Maximize()= 0;
        virtual void Minimize()= 0;
        virtual void Restore() = 0;

        [[nodiscard]] virtual bool IsOpen() const = 0;
        [[nodiscard]] virtual bool IsMinimized() const = 0;
        [[nodiscard]] virtual bool IsFullscreen() const = 0;
        [[nodiscard]] virtual bool IsFocused() const = 0;
        [[nodiscard]] virtual bool IsCursorVisible() const = 0;

        virtual void RequestFocus() = 0;
        virtual void SetFullscreen(bool value) = 0;
        virtual void SetCursorVisible(bool value) = 0;

    protected:
        Window() = default;

        virtual void OnClientSizeChanged();

        uint32_t _id{};
        std::string _title;
    };
}
