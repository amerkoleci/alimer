// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Object.h"
#include "Alimer/Core/Log.h"
#include <memory>

using namespace Alimer;

bool Object::IsInstanceOf(StringId32 type) const
{
    return GetTypeInfo()->IsTypeOf(type);
}

bool Object::IsInstanceOf(const TypeInfo* typeInfo) const
{
    return GetTypeInfo()->IsTypeOf(typeInfo);
}

void Object::RegisterFactory(const TypeInfo* typeInfo, ObjectFactory* factory)
{
    if (!factory)
        return;

    TypeInfoReflection::GetReflection(typeInfo)->SetFactory(factory);
}

SharedPtr<Object> CreateObject(const TypeInfo* typeInfo)
{
    auto factory = TypeInfoReflection::GetReflection(typeInfo)->GetFactory();

    return factory != nullptr ? factory->CreateObject() : nullptr;
}

SharedPtr<Object> Object::CreateObject(StringId32 type)
{
    auto factory = TypeInfoReflection::GetReflection(type)->GetFactory();
    return factory != nullptr ? factory->CreateObject() : nullptr;
}

const PropertyVector& Object::GetProperties() const
{
    return GetTypeInfo()->GetProperties();
}

PropertyInfo* Object::GetProperty(const std::string& name) const
{
    const PropertyVector& properties = GetProperties();
    if (properties.empty())
        return nullptr;

    for (size_t i = 0, count = properties.size(); i < count; ++i)
    {
        PropertyInfo* property = properties[i].Get();
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
    //typeInfoCache().RegisterProperty(type, property);
}

void Object::CopyBaseProperties(const StringId32& type, const StringId32& baseType)
{
#if 0
    // Make sure the types are different, which may not be true if the OBJECT macro has been omitted
    if (type != baseType)
    {
        PropertyVector& properties = typeInfoCache().classProperties[baseType];
        for (size_t i = 0, count = properties.size(); i < count; ++i)
        {
            RegisterProperty(type, properties[i].Get());
        }
    }
#endif // 0

}

void Object::CopyBaseProperty(const StringId32& type, const StringId32& baseType, const std::string& name)
{
#if 0
    // Make sure the types are different, which may not be true if the OBJECT macro has been omitted
    if (type != baseType)
    {
        PropertyVector& properties = typeInfoCache().classProperties[baseType];
        for (size_t i = 0, count = properties.size(); i < count; ++i)
        {
            if (properties[i]->GetName() == name)
            {
                RegisterProperty(type, properties[i].Get());
                break;
            }
        }
    }
#endif // 0
}
