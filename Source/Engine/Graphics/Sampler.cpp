// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Sampler.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
	Sampler::Sampler()
	{

	}

	SamplerRef Sampler::Create(const SamplerDesc& desc)
	{
		ALIMER_ASSERT(gGraphics().IsInitialized());

		return gGraphics().CreateSampler(desc);
	}
}
