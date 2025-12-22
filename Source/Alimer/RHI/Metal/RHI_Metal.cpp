// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "AlimerConfig.h"

#if defined(ALIMER_RHI_METAL)
#include "Core/Log.h"
#include "Core/Vector.h"
#include "Core/UnorderedMap.h"
#include "Core/Hash.h"
#include "RHI/RHI.h"

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <array>
#include <mutex>
#include <deque>
#include <memory>
#include <sstream>

namespace Alimer
{
    namespace
    {
    }

    bool Metal_IsSupported()
    {
        return false;
    }

    RHIFactoryRef Metal_CreateFactory(const RHIFactoryDesc& desc)
    {
        // TODO:
        return nullptr;
    }
}
#endif /* defined(ALIMER_RHI_METAL) */
