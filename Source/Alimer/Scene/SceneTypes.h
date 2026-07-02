// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/RefCounted.h"

namespace Alimer
{
    class Component;
    class Entity;
    class Scene;

    using ComponentRef = SharedPtr<Component>;
    using EntityRef = SharedPtr<Entity>;
    using SceneRef = SharedPtr<Scene>;
}
