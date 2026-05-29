// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Scene/Scene.h"
#include "Alimer/Scene/Entity.h"
//#include "Alimer/Scene/Prefab.h"
#include "Alimer/Core/JobSystem.h"


using namespace Alimer;

void Scene::Register()
{
    static bool registered = false;
    if (registered)
        return;
    registered = true;

    RegisterFactory<Scene>();
    Entity::Register();
}

Scene::Scene(StringView name)
    : _name(name)
{
}

Scene::~Scene()
{
}

void Scene::SetName(const String& name)
{
    _name = name;
}

void Scene::SetName(StringView name)
{
    _name = name;
}
