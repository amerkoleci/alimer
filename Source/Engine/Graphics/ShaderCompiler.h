// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/StringUtils.h"
#include "Graphics/Shader.h"

namespace Alimer::ShaderCompiler
{
    struct ShaderModel
    {
        uint8_t major = 6;
        uint8_t minor = 0;
    };

    struct ShaderCompileOptions {
        std::string source;
        std::string entryPoint = "main";
        std::string fileName = "";
        std::vector<std::string> defines;
        ShaderStage stage;
        ShaderModel shaderModel = { 6, 0 };
    };

    ALIMER_API ShaderRef Compile(const std::string& fileName, ShaderBlobType blobType);
    ALIMER_API ShaderRef Compile(ShaderStage stage, const std::string& fileName, ShaderBlobType blobType);
    ALIMER_API ShaderRef Compile(const ShaderCompileOptions& options, ShaderBlobType blobType);
}
