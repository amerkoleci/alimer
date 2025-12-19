// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Object.h"
#include "Alimer/Core/Log.h"
#include <memory>

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

    /// Object factories mutex.
    std::mutex s_objectFactoryMutex;
    /// Object factories.
    UnorderedMap<StringId32, std::unique_ptr<ObjectFactory>> s_factories;

    Attributes& attributes()
    {
        static Attributes s_attributes;
        return s_attributes;
    }
}

TypeInfo::TypeInfo(const char* typeName_, const TypeInfo* baseTypeInfo_)
    : type(typeName_)
    , typeName(typeName_)
    , baseTypeInfo(baseTypeInfo_)
{
}

bool TypeInfo::IsTypeOf(StringId32 type) const
{
    const TypeInfo* current = this;
    while (current)
    {
        if (current->type == type)
            return true;

        current = current->GetBaseTypeInfo();
    }

    return false;
}

bool TypeInfo::IsTypeOf(const TypeInfo* typeInfo) const
{
    if (typeInfo == nullptr)
        return false;

    const TypeInfo* current = this;
    while (current)
    {
        if (current == typeInfo || current->type == typeInfo->type)
            return true;

        current = current->GetBaseTypeInfo();
    }

    return false;
}

bool Object::IsInstanceOf(StringId32 type) const
{
    return GetTypeInfo()->IsTypeOf(type);
}

bool Object::IsInstanceOf(const TypeInfo* typeInfo) const
{
    return GetTypeInfo()->IsTypeOf(typeInfo);
}

void Object::RegisterFactory(ObjectFactory* factory)
{
    if (!factory)
        return;

    std::lock_guard lockGuard(s_objectFactoryMutex);
    s_factories[factory->GetType()].reset(factory);
}

SharedPtr<Object> Object::CreateObject(StringId32 type)
{
    auto it = s_factories.find(type);
    return it != s_factories.end() ? it->second->CreateObject() : nullptr;
}

const PropertyVector* Object::GetProperties() const
{
    auto it = attributes().classProperties.find(GetType());
    return it != attributes().classProperties.end() ? &it->second : nullptr;
}


PropertyInfo* Object::GetProperty(const std::string& name) const
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

void Object::SetPropertyValue(_In_ PropertyInfo* property, const void* source)
{
    ALIMER_ASSERT(property);

    property->SetValue(this, source);
}

bool Object::SetPropertyValue(const std::string& name, const void* source)
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

void Object::GetPropertyValue(_In_ PropertyInfo* property, void* dest)
{
    ALIMER_ASSERT(property);

    property->GetValue(this, dest);
}

bool Object::GetPropertyValue(const std::string& name, void* dest)
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

void Object::RegisterProperty(const StringId32& type, PropertyInfo* property)
{
    attributes().RegisterProperty(type, property);
}

void Object::CopyBaseProperties(const StringId32& type, const StringId32& baseType)
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

void Object::CopyBaseProperty(const StringId32& type, const StringId32& baseType, const std::string& name)
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

std::string Object::GetTypeNameFromType(StringId32 type)
{
    auto it = s_factories.find(type);
    return it != s_factories.end() ? it->second->GetTypeName() : type.GetString();
}
