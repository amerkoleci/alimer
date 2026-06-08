// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.


#include "Alimer/Scene/Component.h"
#include "Alimer/Scene/Entity.h"
#include "Alimer/Scene/Scene.h"

using namespace Alimer;

Entity* Component::GetEntity() const
{
    return _entity;
}

[[nodiscard]] UUID Component::GetID() const
{
    return _id;
}

[[nodiscard]] Scene* Component::GetScene() const
{
    return _entity ? _entity->GetScene() : nullptr;
}

void Component::SetEntity(Entity* value)
{
    if (_entity == value)
        return;

    _entity = value;
}

void Component::SetID(UUID value)
{
    _id = value;
}

void Component::SetEnabled(bool value)
{
    if (_enabled == value)
        return;

    _enabled = value;

    Scene* scene = GetScene();
    if (scene != nullptr)
    {
        //scene->OnComponentEnabledChanged(this);
    }
}
