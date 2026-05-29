// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/TypeInfo.h"
#include "Alimer/Core/Object.h"
#include "Alimer/Core/Log.h"
#include <memory>
#include <mutex>

using namespace Alimer;

namespace
{
    struct TypeInfoCache
    {
        std::mutex cacheMutex;
        UnorderedMap<StringId32, SharedPtr<TypeInfoReflection>> cache;

        TypeInfoReflection* Register(const TypeInfo* typeInfo)
        {
            if (!typeInfo)
            {
                LOGW("Attempt to reflect class without TypeInfo");
                return nullptr;
            }

            std::lock_guard lockGuard(cacheMutex);
            const StringId32 typeInfoId = typeInfo->GetType();
            const auto iter = cache.find(typeInfoId);
            if (iter == cache.end())
            {
                const auto reflection = MakeShared<TypeInfoReflection>(typeInfo);
                cache.emplace(typeInfoId, reflection);
                return reflection;
            }

            return iter->second.Get();
        }

        TypeInfoReflection* GetReflection(StringId32 typeInfoId)
        {
            std::lock_guard lockGuard(cacheMutex);
            const auto iter = cache.find(typeInfoId);
            return iter != cache.end() ? iter->second.Get() : nullptr;
        }

        const TypeInfo* GetTypeInfo(StringId32 typeInfoId)
        {
            std::lock_guard lockGuard(cacheMutex);
            const auto iter = cache.find(typeInfoId);
            return iter != cache.end() ? iter->second->GetTypeInfo() : nullptr;
        }
    };

    TypeInfoCache& typeInfoCache()
    {
        static TypeInfoCache s_cache;
        return s_cache;
    }
}

TypeInfo::TypeInfo(const char* typeName, const TypeInfo* baseTypeInfo)
    : _type(typeName)
    , _typeName(typeName)
    , _baseTypeInfo(baseTypeInfo)
{
    _reflection = typeInfoCache().Register(this);
}

bool TypeInfo::IsTypeOf(StringId32 type) const
{
    const TypeInfo* current = this;
    while (current)
    {
        if (current->_type == type)
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
        if (current == typeInfo || current->_type == typeInfo->_type)
            return true;

        current = current->GetBaseTypeInfo();
    }

    return false;
}

const PropertyVector& TypeInfo::GetProperties() const
{
    return _reflection->GetProperties();
}

/* TypeInfoReflection */
TypeInfoReflection::TypeInfoReflection(const TypeInfo* typeInfo)
    : _typeInfo(typeInfo)
{

}
void TypeInfoReflection::SetFactory(ObjectFactory* factory)
{
    _factory.reset(factory);
}

bool TypeInfoReflection::RegisterProperty(PropertyInfo* property)
{
    for (size_t i = 0, count = _properties.size(); i < count; ++i)
    {
        if (_properties[i]->GetName() == property->GetName())
        {
            //_properties.insert(_properties.begin() + i, sharedProp);
            return false;
        }
    }

    SharedPtr<PropertyInfo> sharedProp(property);

    _properties.push_back(sharedProp);
    return true;
}

const TypeInfo* TypeInfo::Get(StringId32 typeId)
{
    return typeInfoCache().GetTypeInfo(typeId);
}

TypeInfoReflection* Alimer::GetTypeInfoReflection(StringId32 typeId)
{
    return typeInfoCache().GetReflection(typeId);
}

TypeInfoReflection* Alimer::GetTypeInfoReflection(const TypeInfo* typeInfo)
{
    ALIMER_ASSERT(typeInfo);

    return typeInfoCache().GetReflection(typeInfo->GetType());
}


void Alimer::RegisterFactory(const TypeInfo* typeInfo, ObjectFactory* factory)
{
    if (!factory)
        return;

    GetTypeInfoReflection(typeInfo)->SetFactory(factory);
}

SharedPtr<Object> Alimer::CreateObject(const TypeInfo* typeInfo)
{
    auto factory = GetTypeInfoReflection(typeInfo)->GetFactory();

    return factory != nullptr ? factory->CreateObject() : nullptr;
}

SharedPtr<Object> Alimer::CreateObject(StringId32 type)
{
    auto factory = GetTypeInfoReflection(type)->GetFactory();
    return factory != nullptr ? factory->CreateObject() : nullptr;
}

void Alimer::RegisterProperty(const StringId32& type, PropertyInfo* property)
{
    GetTypeInfoReflection(type)->RegisterProperty(property);
}

void Alimer::CopyBaseProperties(const StringId32& type, const StringId32& baseType)
{
    // Make sure the types are different, which may not be true if the OBJECT macro has been omitted
    if (type != baseType)
    {
        PropertyVector& properties = GetTypeInfoReflection(baseType)->GetProperties();
        for (size_t i = 0, count = properties.size(); i < count; ++i)
        {
            RegisterProperty(type, properties[i].Get());
        }
    }
}

void Alimer::CopyBaseProperty(const StringId32& type, const StringId32& baseType, const std::string& name)
{
    // Make sure the types are different, which may not be true if the OBJECT macro has been omitted
    if (type != baseType)
    {
        PropertyVector& properties = GetTypeInfoReflection(baseType)->GetProperties();
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
