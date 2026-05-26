// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Object.h"
#include "Alimer/IO/Stream.h"
#include "Alimer/Assets/AssetRef.h"

using namespace Alimer;

const AssetRef AssetRef::Empty{ };

AssetRef AssetRef::Parse(const String& str)
{
    AssetRef result;
    if (TryParse(str, &result))
    {
        return result;
    }

    return result;
}

bool AssetRef::TryParse(const String& str, AssetRef* result)
{
    ALIMER_ASSERT(result);

    Vector<std::string> values = StringUtils::SplitNoEmpty(str, ";");
    if (values.size() == 2)
    {
        result->type = values[0];
        result->name = values[1];
        return true;
    }

    return false;
}

void AssetRef::FromBinary(Stream& source)
{
    type = source.ReadStringId32();
    name = source.ReadString();
}

std::string AssetRef::ToString() const
{
    return TypeInfoReflection::GetReflection(type)->GetTypeName() + ";" + name;
}

bool AssetRef::ToBinary(Stream& dest) const
{
    bool success = dest.Write(type);
    success &= dest.Write(name);
    return success;
}

const AssetRefList AssetRefList::Empty{ };

AssetRefList AssetRefList::Parse(const String& str)
{
    AssetRefList result;
    if (TryParse(str, &result))
    {
        return result;
    }

    return result;
}

bool AssetRefList::TryParse(const String& str, AssetRefList* result)
{
    ALIMER_ASSERT(result);

    StringVector values = StringUtils::SplitNoEmpty(str, ";");
    if (values.size() >= 1)
    {
        result->type = values[0];
        result->names.clear();
        for (size_t i = 1; i < values.size(); ++i)
        {
            result->names.push_back(values[i]);
        }
        return true;
    }

    return false;
}

void AssetRefList::FromBinary(Stream& source)
{
    type = source.ReadStringId32();
    uint32_t num = source.ReadVLE();
    names.clear();
    for (uint32_t i = 0; i < num && !source.IsEof(); ++i)
    {
        names.push_back(source.ReadString());
    }
}

std::string AssetRefList::ToString() const
{
    std::string ret(TypeInfoReflection::GetReflection(type)->GetTypeName());
    for (auto it = names.begin(); it != names.end(); ++it)
    {
        ret += ";";
        ret += *it;
    }
    return ret;
}

void AssetRefList::ToBinary(Stream& dest) const
{
    dest.Write(type);
    dest.WriteVLE(names.size());
    for (auto it = names.begin(); it != names.end(); ++it)
        dest.Write(*it);
}
