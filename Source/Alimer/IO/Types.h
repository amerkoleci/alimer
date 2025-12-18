// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"
#include <filesystem>

namespace Alimer
{
    /// File open mode.
    enum class FileMode : uint32_t
    {
        Read,
        Write,
        ReadWrite,
    };

    using FileSystemPath = std::filesystem::path;
}
