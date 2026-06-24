// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Renderer/Types.h"
#include "Alimer/Assets/Asset.h"
#include "Alimer/RHI/RHI.h"

namespace Alimer
{
    class ALIMER_API Mesh final : public Asset
    {
    public:
        static void Register();

        /// Constructor.
        explicit Mesh(std::string_view name = kEmptyStringView);

        /// Destructor.
        ~Mesh() override;

        void Destroy();

    private:
        void ClearCpuData();
    };
}
