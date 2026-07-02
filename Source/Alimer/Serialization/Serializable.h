// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Containers.h"
#include "Alimer/Core/Object.h"
#include "Alimer/Core/Property.h"

namespace Alimer
{
    class ObjectResolver;

    /// Base class for objects with automatic serialization using properties.
    class ALIMER_API Serializable : public Object
    {
        ALIMER_OBJECT(Serializable, Object);

    public:
        /// Load from binary stream. Store object ref attributes to be resolved later.
        virtual void Load(Stream& source, ObjectResolver& resolver);

        /// Save to binary stream.
        virtual void Save(Stream& dest);

        /// Load from serialized data. Optionally store object ref attributes to be resolved later.
        virtual void Load(const SerializeValue& source, ObjectResolver& resolver);

        /// Save as serializable value.
        virtual void Save(SerializeValue& dest);
    };
}
