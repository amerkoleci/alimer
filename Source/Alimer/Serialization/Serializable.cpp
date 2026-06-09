// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/IO/Stream.h"
#include "Alimer/Serialization/Serializable.h"
#include "Alimer/Serialization/Serializer.h"

using namespace Alimer;

#if TODO_SERIALIAZION
void Serializable::Serialize(ISerializer& serializer)
{
    const PropertyVector* properties = GetProperties();
    if (properties == nullptr || properties->empty())
        return;
    
    for (size_t i = 0, count = properties->size(); i < count; ++i)
    {
        PropertyInfo* property = properties->at(i).Get();
        if (property->IsDefault(this))
            continue;

        property->Serialize(this, serializer);
    }
}

void Serializable::Deserialize(IDeserializer& deserializer)
{
    const PropertyVector* properties = GetProperties();
    if (properties == nullptr || properties->empty())
        return;

    for (size_t i = 0, count = properties->size(); i < count; ++i)
    {
        PropertyInfo* property = properties->at(i).Get();
        property->Deserialize(this, deserializer);
    }
}
#endif

void Serializable::Load(Stream& source/*, ObjectResolver& resolver*/)
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
                //if (type != VariantType::ObjectRef)
                property->FromBinary(this, source);
                //else
                //    resolver.StoreObjectRef(this, property, source.Read<ObjectRef>());

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
