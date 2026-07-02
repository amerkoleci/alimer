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

bool Entity::IsRoot() const
{
    return _parent == nullptr;
}

void Entity::OnTransformChanged()
{
}

void Entity::MarkDirty(bool markLocalDirty)
{
    if (markLocalDirty)
        _localMatrixDirty = true;

    OnTransformChanged();
    if (_worldMatrixDirty)
        return;

    _worldMatrixDirty = true;
    for (auto& child : _children)
    {
        child->MarkDirty(false);
    }
}

void Entity::SetLocalPosition(float x, float y, float z)
{
    _localPosition.Set(x, y, z);
    MarkDirty();
}

void Entity::SetLocalPosition(const Vector3& position)
{
    _localPosition = position;
    MarkDirty();
}
