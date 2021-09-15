// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Shader.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
	Shader::Shader(ShaderStages stage, const std::vector<uint8_t>& byteCode_, const std::string& entryPoint_)
        : stage{ stage }
		, byteCode(byteCode_)
		, entryPoint(entryPoint_)
	{

	}

	ShaderRef Shader::Create(ShaderStages stage, const std::vector<uint8_t>& byteCode, const std::string& entryPoint)
	{
		ALIMER_ASSERT(gGraphics().IsInitialized());
		ALIMER_ASSERT(byteCode.size() > 0);

		return gGraphics().CreateShader(stage, byteCode, entryPoint);
	}
}
