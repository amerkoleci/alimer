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

const PropertyVector& Object::GetProperties() const
{
    return GetTypeInfo()->GetProperties();
}

PropertyInfo* Object::GetProperty(StringView name) const
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

void Object::GetPropertyValue(_In_ PropertyInfo* property, void* dest)
{
    ALIMER_ASSERT(property);

    property->GetValue(this, dest);
}

bool Object::GetPropertyValue(StringView name, void* dest)
{
    auto property = GetProperty(name);
    if (property == nullptr)
    {
        LOGE("Could not find property {} in {}", name, GetTypeName());
        return false;
    }

    property->GetValue(this, dest);
    return true;
}


void Object::SetPropertyValue(_In_ PropertyInfo* property, const void* source)
{
    ALIMER_ASSERT(property);

    property->SetValue(this, source);
}

bool Object::SetPropertyValue(StringView name, const void* source)
{
    auto property = GetProperty(name);
    if (property == nullptr)
    {
        LOGE("Could not find property {} in {}", name, GetTypeName());
        return false;
    }

    property->SetValue(this, source);
    return true;
}

bool Object::SetPropertyValue(StringView name, StringView value)
{
    auto property = GetProperty(name);
    if (property != nullptr)
    {
        VariantType propertyType = property->GetPropertyType();

        if (propertyType == VariantType::String)
        {
            std::string stringValue(value);
            property->SetValue(this, &stringValue);
            return true;
        }
    }

    return false;
}

bool Object::SetPropertyValue(StringView name, const char* value)
{
    auto property = GetProperty(name);
    if (property != nullptr)
    {
        VariantType propertyType = property->GetPropertyType();
        if (propertyType == VariantType::String)
        {
            std::string stringValue(value);
            property->SetValue(this, &stringValue);
            return true;
        }
    }

    return false;
}
