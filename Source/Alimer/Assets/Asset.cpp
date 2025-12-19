// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/IO/Stream.h"
#include "Alimer/Assets/Asset.h"

using namespace Alimer;

Asset::Asset(std::string_view name_)
{
    if (!name_.empty())
    {
        SetName(name_);
    }
}

bool Asset::Load(Stream& source)
{
    bool success = BeginLoad(source);
    if (success)
        success &= EndLoad();

    return success;
}

bool Asset::Save(Stream&) const
{
    LOGE("Save not supported for {}", GetTypeName());
    return false;
}

bool Asset::BeginLoad(Stream&)
{
    return false;
}

bool Asset::EndLoad()
{
    return true;
}

void Asset::SetName(std::string_view name_)
{
    name = name_;
    nameId = StringId32(name);
}

void Asset::SetMemoryUse(uint32_t size)
{
    memoryUse = size;
}
