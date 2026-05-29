// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Scene/Entity.h"
#include "Alimer/Scene/Scene.h"

using namespace Alimer;

void Entity::Register()
{
    RegisterFactory<Entity>();
    CopyBaseProperties<Entity, Serializable>();
    RegisterRefProperty("name", &Entity::GetName, &Entity::SetName, kEmptyString);
}

Entity::Entity(StringView name)
    : _name(name)
{

}

Entity::~Entity()
{
    //RemoveAllChildren();
    //RemoveAllComponents();

    // Remove from the scene
    //if (_scene)
    //    _scene->OnEntityRemoved(this);
}

void Entity::SetName(const String& name)
{
    _name = name;
}

void Entity::SetName(StringView name)
{
    _name = name;
}
