// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsResource.h"

namespace Alimer
{
    class ALIMER_API Shader : public RefCounted
    {
    public:
        /// Create new shader.
        [[nodiscard]] static ShaderRef Create(ShaderStages stage, const std::string& source, const std::string& entryPoint = "main");

    protected:
        /// Constructor.
        Shader(ShaderStages stage);

        ShaderStages stage;
    };
}

