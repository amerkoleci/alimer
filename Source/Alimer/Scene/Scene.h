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
        friend class SceneSystem;

    private:
        /// Register factory and attributes.
        static void Register();

    public:
        Scene(StringView name = kEmptyStringView);
        ~Scene() override;

        [[nodiscard]] const String& GetName() const { return _name; }
        void SetName(const String& name);
        void SetName(StringView name);

    private:
        String _name;
    };
}
