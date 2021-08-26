// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Types.h"
#include "Math/Color.h"
#include "Graphics/Types.h"

namespace alimer
{
    enum class ClearMask : uint32_t
    {
        None = 0,
        Color = 1,
        Depth = 2,
        Stencil = 4,
        All = (int)Color | (int)Depth | (int)Stencil
    };
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(ClearMask);

    class ALIMER_API CommandList
    {
    protected:
        CommandList() = default;
        virtual ~CommandList() = default;

    public:
        // Non-copyable and non-movable
        CommandList(const CommandList&) = delete;
        CommandList(const CommandList&&) = delete;
        CommandList& operator=(const CommandList&) = delete;
        CommandList& operator=(const CommandList&&) = delete;

        virtual void PushDebugGroup(const std::string_view& name) = 0;
        virtual void PopDebugGroup() = 0;
        virtual void InsertDebugMarker(const std::string_view& name) = 0;

        virtual void BeginDefaultRenderPass(const Color& clearColor, float clearDepth = 1.0f, uint8_t clearStencil = 0, ClearMask mask = ClearMask::All) = 0;
        virtual void EndRenderPass() = 0;
    };
}

