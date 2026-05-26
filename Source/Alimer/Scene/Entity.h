// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Scene/Component.h"

namespace Alimer
{
    static constexpr uint32_t kSmallSubtaskGroupSize = 64;

    class ALIMER_API Entity final : public Serializable
    {
        ALIMER_OBJECT(Entity, Serializable);

        friend class Component;
        friend class Scene;

        /// Register factory and attributes (called by Scene::Register).
        static void Register();

    public:
        Entity(std::string_view name = kEmptyStringView);
        ~Entity() override;

        [[nodiscard]] std::string GetName() const { return _name; }
        void SetName(std::string_view name);

    private:
        std::string _name;
    };
}
