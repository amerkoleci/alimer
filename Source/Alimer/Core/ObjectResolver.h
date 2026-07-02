// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Containers.h"
#include <map>

namespace Alimer
{
    class PropertyInfo;
    class Object;
    struct ObjectRef;

    /// Stored object ref attribute.
    struct StoredObjectRef
    {
        /// Construct undefined.
        StoredObjectRef() noexcept = default;

        /// Construct with values.
        StoredObjectRef(Object* object_, PropertyInfo* property_, uint32_t oldId_)
            : object(object_)
            , property(property_)
            , oldId(oldId_)
        {
        }

        /// %Object that contains the attribute.
        Object* object = nullptr;
        /// Description of the object ref attribute.
        PropertyInfo* property = nullptr;
        /// Old id from the serialized data.
        uint32_t oldId = 0;
    };

    /// Helper class for resolving object ref attributes when loading a scene.
    class ObjectResolver
    {
    public:
        /// Store an object along with its old id from the serialized data.
        void StoreObject(uint32_t oldId, Object* object);
        /// Store an object ref attribute that needs to be resolved later.
        void StoreObjectRef(Object* object, PropertyInfo* property, const ObjectRef& value);
        /// Resolve the object ref attributes.
        void Resolve();

    private:
        /// Mapping of old id's to objects.
        std::map<uint32_t, Object*> _objects;
        /// Stored object ref attributes.
        Vector<StoredObjectRef> _objectRefs;
    };
}
