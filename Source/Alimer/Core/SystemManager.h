// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/System.h"
#include "Alimer/Core/StringId.h"
#include "Alimer/Core/Vector.h"
#include "Alimer/Core/UnorderedMap.h"
#include <mutex>

namespace Alimer
{
    /// Class that manages the systems
    class ALIMER_API SystemManager
    {
    public:
        SystemManager() = default;
        ~SystemManager();

        /// Clears up all registered systems.
        void Clear();

        /// Setup and orders the systems based on their priority. This should be called after all systems have been added and before the main loop starts.
        void Setup();

        template<typename T, typename... Args>
        T* AddSystem(Args&&... args)
        {
            std::lock_guard lockGuard(_subsystemsMutex);
            T* system = new T(std::forward<Args>(args)...);
            _subsystems[T::GetTypeStatic()] = system;
            system->Register();
            return system;
        }

        template<typename T>
        T* GetSystem()
        {
            return static_cast<T*>(_subsystems[T::GetTypeStatic()]);
        }

    private:
        std::mutex _subsystemsMutex;
        UnorderedMap<StringId32, System*> _subsystems;
        Vector<System*> _updateList;
    };
}
