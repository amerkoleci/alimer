// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Serialization/Serializable.h"

namespace Alimer
{
    class Entity;
    class Scene;

    /// Base class for Entity components.
    class ALIMER_API Component : public Serializable
    {
        ALIMER_OBJECT(Component, Serializable);

        friend class Entity;
        friend class Scene;

    public:
        Component();
    };
}
