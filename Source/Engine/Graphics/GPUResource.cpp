// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/GPUResource.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

namespace Alimer
{
    void GPUResource::OnCreated()
    {
        gGraphics().AddGPUObject(this);
    }

    void GPUResource::OnDestroyed()
    {
        gGraphics().RemoveGPUObject(this);
    }
}
