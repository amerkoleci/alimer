// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Assets/Asset.h"
#include "IO/Stream.h"
#include "Core/Log.h"

namespace Alimer
{

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

    void Asset::SetName(const std::string& newName)
    {
        name = newName;
        nameId = StringId32(newName);
    }
}
