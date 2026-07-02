// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/ObjectResolver.h"
#include "Alimer/Core/Object.h"
#include "Alimer/Core/Log.h"

using namespace Alimer;

void ObjectResolver::StoreObject(uint32_t oldId, Object* object)
{
    if (object)
    {
        _objects[oldId] = object;
    }
}

void ObjectResolver::StoreObjectRef(Object* object, PropertyInfo* property, const ObjectRef& value)
{
    if (object && property)
    {
        _objectRefs.push_back(StoredObjectRef(object, property, value.id));
    }
}

void ObjectResolver::Resolve()
{
    for (auto it = _objectRefs.begin(); it != _objectRefs.end(); ++it)
    {
        auto refIt = _objects.find(it->oldId);
        // See if we can find the referred object
        if (refIt != _objects.end())
        {
            PropertyInfoImpl<ObjectRef>* typedProperty = static_cast<PropertyInfoImpl<ObjectRef>*>(it->property);
            typedProperty->SetValue(it->object, ObjectRef(refIt->second->GetId()));
        }
        else
        {
            LOGW("Could not resolve object reference {}", it->oldId);
        }
    }
}
