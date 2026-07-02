// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Assets/JsonFile.h"
#include "Alimer/IO/Stream.h"
#include "Alimer/Private/rapidjson_wrapper.h"

using namespace Alimer;

void JsonFile::Register()
{
    RegisterFactory<JsonFile>();
}

JsonFile::JsonFile(std::string_view name)
    : Asset(name)
{
}

bool JsonFile::FromString(const std::string& source)
{
    if (source.empty())
        return false;

    return root.FromJson(source);
}

std::string JsonFile::ToString(const std::string& indendation) const
{
    return root.ToJson(indendation);
}

bool JsonFile::BeginLoad(Stream& source)
{
    const size_t dataSize = source.GetSize() - source.GetPosition();
    std::unique_ptr<char[]> buffer(new char[dataSize]);
    if (source.Read(buffer.get(), dataSize) != dataSize)
        return false;

    if (!root.FromJson(buffer.get(), true))
    {
        return false;
    }

    return true;
}

bool JsonFile::Save(Stream& stream) const
{
    std::string buffer;
    root.ToJson(buffer);
    if (buffer.length())
    {
        return stream.Write(&buffer[0], buffer.length()) == buffer.length();
    }

    return true;
}
