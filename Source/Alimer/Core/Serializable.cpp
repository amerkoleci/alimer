// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/IO/Stream.h"
#include "Alimer/Core/ObjectResolver.h"
#include "Alimer/Core/Serializable.h"

using namespace Alimer;

void Serializable::Load(Stream& source, ObjectResolver& resolver)
{
    const PropertyVector& properties = GetProperties();
    if (properties.empty())
        return;

    //StringId32 type = source.ReadStringId32();
    size_t propertyCount = source.ReadVLE();
    for (size_t i = 0; i < propertyCount; ++i)
    {
        // Skip attribute if wrong type or extra data
        VariantType type = source.ReadEnum<VariantType>();
        bool skip = true;

        if (i < properties.size())
        {
            PropertyInfo* property = properties[i].Get();
            if (property->GetPropertyType() == type)
            {
                // Store object refs to the resolver instead of immediately setting
                if (type != VariantType::ObjectRef)
                {
                    property->FromBinary(this, source);
                }
                else
                {
                    resolver.StoreObjectRef(this, property, source.ReadObjectRef());
                }

                skip = false;
            }
        }

        if (skip)
            PropertyInfo::Skip(type, source);
    }
}

void Serializable::Save(Stream& dest)
{
    const PropertyVector& properties = GetProperties();
    if (properties.empty())
        return;

    //dest.Write(GetType());
    dest.WriteVLE(properties.size());
    for (size_t i = 0, count = properties.size(); i < count; ++i)
    {
        PropertyInfo* property = properties[i].Get();

        dest.Write(property->GetPropertyType());
        property->ToBinary(this, dest);
    }
}

void Serializable::Load(const SerializeValue& source, ObjectResolver& resolver)
{
    const PropertyVector& properties = GetProperties();
    if (properties.empty() || !source.IsObject() || !source.Size())
        return;

    const SerializeValueObject& object = source.GetObject();

    for (auto it = properties.begin(); it != properties.end(); ++it)
    {
        PropertyInfo* property = *it;
        auto jsonIt = object.find(property->GetName());
        if (jsonIt != object.end())
        {
            // Store object refs to the resolver instead of immediately setting
            if (property->GetPropertyType() != VariantType::ObjectRef)
            {
                property->FromSerializeValue(this, jsonIt->second);
            }
            else
            {
                resolver.StoreObjectRef(this, property, ObjectRef(jsonIt->second.GetUInt32()));
            }
        }
    }
}

void Serializable::Save(SerializeValue& dest)
{
    const PropertyVector& properties = GetProperties();
    if (properties.empty())
        return;

    for (size_t i = 0, count = properties.size(); i < count; ++i)
    {
        PropertyInfo* property = properties[i].Get();
        // For better readability, do not save default-valued attributes to serialize value.
        if (!property->IsDefault(this))
        {
            property->ToSerializeValue(this, dest[property->GetName()]);
        }
    }
}
