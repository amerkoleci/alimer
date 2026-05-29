// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Scene/SceneSystem.h"

using namespace Alimer;

SceneSystem::SceneSystem()
{
    _currentScene = new Scene();
    //_currentScene->OnInit();
}

SceneSystem::~SceneSystem()
{
}

void SceneSystem::Register()
{
    Scene::Register();
}

void SceneSystem::Unregister()
{

}
