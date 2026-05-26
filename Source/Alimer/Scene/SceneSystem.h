// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/System.h"
#include "Alimer/Scene/Scene.h"

namespace Alimer
{
    /// Manages the current scene, including loading, saving, and transitioning between scenes.
    class SceneSystem : public System
    {
    public:
        SceneSystem();
        ~SceneSystem() override;

        void Register() override;
        void Unregister() override; 

        std::string GetName() const override { return "Scene System"; }
        int GetPriority() const override { return 0; }

    private:
        SceneRef _currentScene;
    };
}
