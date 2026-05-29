// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Object.h"

namespace Alimer
{
    /// Base class for an engine system.
    class ALIMER_API System : public Object
    {
        ALIMER_OBJECT(System, Object);

    public:
        virtual ~System() = default;

        /// Register any types or components that the system needs.
        virtual void Register() {};

        /// Unregister any types or components that the system needs.
        virtual void Unregister() {};

        /// Get the name of the system.
        virtual std::string GetName() const = 0;

        /// Get the priority of the system.
        virtual int GetPriority() const = 0;
    };
}
