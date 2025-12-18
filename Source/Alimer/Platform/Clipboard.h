// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Platform/Types.h"
#include <memory>

namespace Alimer::Clipboard
{
    ALIMER_API std::string GetText();
    ALIMER_API void SetText(const std::string& value);
}
