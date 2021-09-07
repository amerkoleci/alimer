// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Shader.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
    Shader::Shader(ShaderStages stage)
        : stage{ stage }
    {
    }

    ShaderRef Shader::Create(ShaderStages stage, const std::string& source, const std::string& entryPoint)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

        return gGraphics().CreateShader(stage, source, entryPoint);
    }
}
