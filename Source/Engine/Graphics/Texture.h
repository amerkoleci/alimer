// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Types.h"
#include "Core/RefCount.h"

namespace alimer
{
    class Texture;
    using TextureRef = RefCountPtr<Texture>;

    class ALIMER_API Texture : public RefCounted
    {
    public:
        static TextureRef Create(u32 width, u32 height);
    };
}

