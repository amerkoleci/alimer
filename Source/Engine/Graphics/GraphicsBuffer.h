// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "RHI/RHI.h"
#include "Graphics/GraphicsResource.h"
#include <unordered_map>

namespace Alimer
{
    class ALIMER_API GraphicsBuffer : public GraphicsResource
    {
    public:
        GraphicsBuffer();
        ~GraphicsBuffer() override;

        [[nodiscard]] bool Create(const BufferDesc& desc, const void* initialData = nullptr);

        uint64_t GetAllocatedSize() const override { return allocatedSize; }

    protected:
        /// Constructor.
        GraphicsBuffer(const BufferDesc& info);

        u64 allocatedSize = 0;
        GPUBuffer handle{};
    };
}
