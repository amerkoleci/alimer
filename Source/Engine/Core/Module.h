// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include <utility>

namespace alimer
{
    template<class T>
    class Module
    {
    public:
        static T& Instance()
        {
            return *_Instance();
        }

        static T* InstancePtr()
        {
            return _Instance();
        }

        template<typename Derived, typename ...Args>
        static void Start(Args &&...args)
        {
            _Instance() = new Derived(std::forward<Args>(args)...);
        }

        template<typename ...Args>
        static void Start(Args &&...args)
        {
            _Instance() = new T(std::forward<Args>(args)...);
        }

        void Shutdown()
        {
            delete _Instance();
        }

        bool IsInitialized() const
        {
            return _Instance() != nullptr;
        }

    protected:
        Module()
        {
        }

        virtual ~Module() = default;

        Module(Module&&) = delete;
        Module(const Module&) = delete;
        Module& operator=(Module&&) = delete;
        Module& operator=(const Module&) = delete;

        /** Returns a singleton instance of this module. */
        static T*& _Instance()
        {
            static T* inst = nullptr;
            return inst;
        }
    };
}
