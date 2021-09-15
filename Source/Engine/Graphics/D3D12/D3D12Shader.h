// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Shader.h"
#include "D3D12Utils.h"

namespace Alimer
{
	class D3D12Shader final : public Shader
	{
	public:
		D3D12Shader(D3D12Graphics& device, ShaderStages stage, const std::vector<uint8_t>& byteCode, const std::string& entryPoint);
		~D3D12Shader() override;
		void Destroy() override;

	private:
		D3D12Graphics& device;
	};
}

