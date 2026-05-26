// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/IO/Stream.h"
#include "Alimer/Serialization/Serializable.h"
#include "Alimer/Serialization/Serializer.h"

using namespace Alimer;

namespace
{
    struct Attributes
    {
        UnorderedMap<StringId32, PropertyVector> classProperties;

        void RegisterProperty(const StringId32& type, PropertyInfo* property)
        {
            SharedPtr<PropertyInfo> sharedProp(property);

            PropertyVector& properties = classProperties[type];
            for (size_t i = 0, count = properties.size(); i < count; ++i)
            {
                if (properties[i]->GetName() == property->GetName())
                {
                    properties.insert(properties.begin() + i, sharedProp);
                    return;
                }
            }

            properties.push_back(sharedProp);
        }
    };

    Attributes& attributes()
    {
        static Attributes s_attributes;
        return s_attributes;
    }
}

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
    const PropertyVector* properties = GetProperties();
    if (properties == nullptr || properties->empty())
        return;

    //StringId32 type = source.ReadStringId32();
    size_t numAttrs = source.ReadVLE();
    for (size_t i = 0; i < numAttrs; ++i)
    {
        // Skip attribute if wrong type or extra data
        VariantType type = (VariantType)source.ReadUInt8();
        bool skip = true;

        if (i < properties->size())
        {
            PropertyInfo* property = properties->at(i);
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
    const PropertyVector* properties = GetProperties();
    if (properties == nullptr || properties->empty())
        return;

    dest.Write(GetType());
    dest.WriteVLE(properties->size());
    for (size_t i = 0, count = properties->size(); i < count; ++i)
    {
        PropertyInfo* property = properties->at(i).Get();

        dest.Write((uint8_t)property->GetPropertyType());
        property->ToBinary(this, dest);
    }
}

const PropertyVector* Serializable::GetProperties() const
{
    auto it = attributes().classProperties.find(GetType());
    return it != attributes().classProperties.end() ? &it->second : nullptr;
}

PropertyInfo* Serializable::GetProperty(const std::string& name) const
{
    const PropertyVector* properties = GetProperties();
    if (!properties || properties->empty())
        return nullptr;

    for (size_t i = 0, count = properties->size(); i < count; ++i)
    {
        PropertyInfo* property = properties->at(i).Get();
        if (property->GetName() == name)
        {
            return property;
        }
    }

    return nullptr;
}

void Serializable::SetPropertyValue(_In_ PropertyInfo* property, const void* source)
{
    ALIMER_ASSERT(property);

    property->SetValue(this, source);
}

bool Serializable::SetPropertyValue(const std::string& name, const void* source)
{
    auto property = GetProperty(name);
    if (property == nullptr)
    {
        LOGE("Could not find attribute {} in {}", name, GetTypeName());
        return false;
    }

    property->SetValue(this, source);
    return true;
}

void Serializable::GetPropertyValue(_In_ PropertyInfo* property, void* dest)
{
    ALIMER_ASSERT(property);

    property->GetValue(this, dest);
}

bool Serializable::GetPropertyValue(const std::string& name, void* dest)
{
    auto property = GetProperty(name);
    if (property == nullptr)
    {
        LOGE("Could not find attribute {} in {}", name, GetTypeName());
        return false;
    }

    property->GetValue(this, dest);
    return true;
}


void Serializable::RegisterProperty(const StringId32& type, PropertyInfo* property)
{
    attributes().RegisterProperty(type, property);
}

void Serializable::CopyBaseProperties(const StringId32& type, const StringId32& baseType)
{
    // Make sure the types are different, which may not be true if the OBJECT macro has been omitted
    if (type != baseType)
    {
        PropertyVector& properties = attributes().classProperties[baseType];
        for (size_t i = 0, count = properties.size(); i < count; ++i)
        {
            RegisterProperty(type, properties[i].Get());
        }
    }
}

void Serializable::CopyBaseProperty(const StringId32& type, const StringId32& baseType, const std::string& name)
{
    // Make sure the types are different, which may not be true if the OBJECT macro has been omitted
    if (type != baseType)
    {
        PropertyVector& properties = attributes().classProperties[baseType];
        for (size_t i = 0, count = properties.size(); i < count; ++i)
        {
            if (properties[i]->GetName() == name)
            {
                RegisterProperty(type, properties[i].Get());
                break;
            }
        }
    }
}
