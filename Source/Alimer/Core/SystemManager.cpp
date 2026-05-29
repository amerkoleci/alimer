// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/SystemManager.h"
#include "Alimer/Core/Log.h"

using namespace Alimer;

SystemManager::~SystemManager()
{
    Clear();
}

void SystemManager::Clear()
{
    for (auto [_, system] : _subsystems)
    {
        system->Unregister();
    }
    for (auto [_, system] : _subsystems) {
        delete system;
    }
    _subsystems.clear();
}

void SystemManager::Setup()
{
    // Create list of systems sorted on priority
    _updateList.clear();
    for (auto [_, system] : _subsystems)
    {
        _updateList.push_back(system);
    }

    std::sort(_updateList.begin(), _updateList.end(), [](const auto& a, const auto& b) {
        return a->GetPriority() < b->GetPriority();
        });
}
