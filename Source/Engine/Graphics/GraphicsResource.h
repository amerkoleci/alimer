// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/RefCount.h"
#include "Graphics/GraphicsDefs.h"

namespace Alimer
{
    class ALIMER_API GraphicsResource : public RefCounted
    {
    public:
        enum class Type
        {
            Buffer,
            Texture,
        };

        [[nodiscard]] virtual uint64_t GetAllocatedSize() const = 0;

    protected:
        /// Constructor.
        GraphicsResource(Type type)
            : type{ type }
        {

        }

        Type type;
    };
}

