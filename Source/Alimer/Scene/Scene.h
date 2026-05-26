// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Scene/Entity.h"

namespace Alimer
{
    class Scene;
    using SceneRef = SharedPtr<Scene>;

    class ALIMER_API Scene final : public Serializable
    {
        ALIMER_OBJECT(Scene, Serializable);

        friend class Entity;

        /// Register factory and attributes.
        static void Register();

    public:
        Scene(std::string_view name = kEmptyStringView);
        ~Scene() override;

        [[nodiscard]] std::string GetName() const { return _name; }
        void SetName(std::string_view name);

    private:
        std::string _name;
    };
}
