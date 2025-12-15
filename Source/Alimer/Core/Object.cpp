// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Object.h"
#include "Alimer/Core/Log.h"
#include <memory>

using namespace Alimer;

namespace
{
    /// Object factories mutex.
    std::mutex s_objectFactoryMutex;
    /// Object factories.
    UnorderedMap<StringId32, std::unique_ptr<ObjectFactory>> s_factories;
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

std::string Object::GetTypeNameFromType(StringId32 type)
{
    auto it = s_factories.find(type);
    return it != s_factories.end() ? it->second->GetTypeName() : type.GetString();
}
