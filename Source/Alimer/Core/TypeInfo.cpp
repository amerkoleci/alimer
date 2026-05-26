// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/TypeInfo.h"
#include "Alimer/Core/Object.h"
#include "Alimer/Core/Log.h"
#include <memory>

using namespace Alimer;

namespace
{
    struct TypeInfoCache
    {
        /// Object factories mutex.
        std::mutex s_objectFactoryMutex;
        UnorderedMap<StringId32, SharedPtr<TypeInfoReflection>> typeInfoReflections;

        TypeInfoReflection* Register(const TypeInfo* typeInfo)
        {
            if (!typeInfo)
            {
                LOGW("Attempt to reflect class without TypeInfo");
                return nullptr;
            }

            std::lock_guard lockGuard(s_objectFactoryMutex);
            const StringId32 typeNameId = typeInfo->GetType();
            const auto iter = typeInfoReflections.find(typeNameId);
            if (iter == typeInfoReflections.end())
            {
                const auto reflection = MakeShared<TypeInfoReflection>(typeInfo);
                typeInfoReflections.emplace(typeNameId, reflection);
                return reflection;
            }

            return iter->second.Get();
        }

        TypeInfoReflection* GetReflection(StringId32 typeInfoId)
        {
            std::lock_guard lockGuard(s_objectFactoryMutex);
            const auto iter = typeInfoReflections.find(typeInfoId);
            return iter != typeInfoReflections.end() ? iter->second.Get() : nullptr;
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
    if (baseTypeInfo)
    {
        //typeInfoCache().Register(baseTypeInfo);
    }

    _reflection = typeInfoCache().Register(this);
}

const TypeInfo* TypeInfo::GetTypeInfo(StringId32 typeId)
{
    TypeInfoReflection* reflection = typeInfoCache().GetReflection(typeId);

    return reflection ? reflection->GetTypeInfo() : nullptr;
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

void TypeInfoReflection::RegisterProperty(PropertyInfo* property)
{
    SharedPtr<PropertyInfo> sharedProp(property);

    for (size_t i = 0, count = _properties.size(); i < count; ++i)
    {
        if (_properties[i]->GetName() == property->GetName())
        {
            _properties.insert(_properties.begin() + i, sharedProp);
            return;
        }
    }

    _properties.push_back(sharedProp);
}

TypeInfoReflection* TypeInfoReflection::GetReflection(StringId32 typeId)
{
    const auto iter = typeInfoCache().typeInfoReflections.find(typeId);
    return iter != typeInfoCache().typeInfoReflections.end() ? iter->second.Get() : nullptr;
}

TypeInfoReflection* TypeInfoReflection::GetReflection(const TypeInfo* typeInfo)
{
    ALIMER_ASSERT(typeInfo);

    return typeInfoCache().GetReflection(typeInfo->GetType());
}
