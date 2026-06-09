// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Vector.h"
#include "Alimer/Core/Object.h"
#include "Alimer/Core/Property.h"

namespace Alimer
{
    class ISerializer;
    class IDeserializer;


    /// Base class for objects with automatic serialization using properties.
    class ALIMER_API Serializable : public Object
    {
        ALIMER_OBJECT(Serializable, Object);

    public:
        /// Serialize.
        //virtual void Serialize(ISerializer& serializer);

        /// Deserialize.
        //virtual void Deserialize(IDeserializer& deserializer);

        /// Load from binary stream. Store object ref attributes to be resolved later.
        virtual void Load(Stream& source/*, ObjectResolver& resolver*/);

        /// Save to binary stream.
        virtual void Save(Stream& dest);
    };
}
