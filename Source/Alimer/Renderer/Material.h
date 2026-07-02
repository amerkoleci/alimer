// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Containers.h"
#include "Alimer/Renderer/Types.h"
#include "Alimer/Assets/Asset.h"

namespace Alimer
{
    class ALIMER_API Material : public Asset
    {
        ALIMER_OBJECT(Material, Asset);

    public:
        static void Register();

        /// Constructor.
        explicit Material(StringView name = kEmptyStringView);

        [[nodiscard]] bool IsTransparent() const { return _alphaMode == MaterialAlphaMode::Blend; }
        [[nodiscard]] MaterialAlphaMode GetAlphaMode() const { return _alphaMode; }
        void SetAlphaMode(MaterialAlphaMode value) { _alphaMode = value; }

        [[nodiscard]] bool IsDoubleSided() const { return _doubleSided; }
        void SetDoubleSided(bool value) { _doubleSided = value; }

        [[nodiscard]] float GetAlphaCutoff() const { return _alphaCutoff; }
        void SetAlphaCutoff(float value) { _alphaCutoff = value; }

    protected:

        MaterialAlphaMode _alphaMode = MaterialAlphaMode::Opaque;
        bool _doubleSided = false;
        float _alphaCutoff = 0.5f;
    };

    class ALIMER_API UnlitMaterial : public Material
    {
        ALIMER_OBJECT(UnlitMaterial, Material);

    public:
        /// Constructor.
        explicit UnlitMaterial(StringView name = kEmptyStringView);

        [[nodiscard]] const Color& GetBaseColor() const { return _baseColor; }
        void SetBaseColor(const Color& value) { _baseColor = value; }

        [[nodiscard]] RHITexture* GetBaseColorTexture() const { return _baseColorMap.resource.Get(); }
        void SetBaseColorTexture(RHITexture* texture, MaterialTextureUVChannel channel = MaterialTextureUVChannel::UV0);
        void SetBaseColorTexture(const String& uri, MaterialTextureUVChannel channel = MaterialTextureUVChannel::UV0);

    protected:
        MaterialTextureMap _baseColorMap;
        Color _baseColor = Colors::White;
    };

    class ALIMER_API PhysicallyBasedMaterial final : public UnlitMaterial
    {
        ALIMER_OBJECT(PhysicallyBasedMaterial, UnlitMaterial);

    public:
        /// Constructor.
        explicit PhysicallyBasedMaterial(StringView name = kEmptyStringView);

        const Color& GetSpecularColor() const { return _specularColor; }
        void SetSpecularColor(const Color& value) { _specularColor = value; }

        [[nodiscard]] const Vector3& GetEmissiveColor() const { return _emissiveColor; }
        void SetEmissiveColor(const Vector3& value) { _emissiveColor = value; }

        [[nodiscard]] float GetEmissiveStrength() const { return _emissiveStrength; }
        void SetEmissiveStrength(float value) { _emissiveStrength = value; }

        [[nodiscard]] float GetRoughness() const { return _roughness; }
        void SetRoughness(float value) { _roughness = value; }

        [[nodiscard]] float GetReflectance() const { return _reflectance; }
        void SetReflectance(float value) { _reflectance = value; }

        [[nodiscard]] float GetMetalness() const { return _metalness; }
        void SetMetalness(float value) { _metalness = value; }

        [[nodiscard]] float GetTransmission() const { return _transmission; }
        void SetTransmission(float value) { _transmission = value; }

        void SetTexture(MaterialTextureSlot slot, RHITexture* texture, MaterialTextureUVChannel channel = MaterialTextureUVChannel::UV0);
        void SetTexture(MaterialTextureSlot slot, const std::string& uri, MaterialTextureUVChannel channel = MaterialTextureUVChannel::UV0);

    private:
        Color _specularColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        Vector3 _emissiveColor{ 0.0f, 0.0f, 0.0f };
        float _emissiveStrength = 1.0f;
        float _roughness = 0.2f;
        float _metalness = 0.0f;
        float _reflectance = 0.04f;
        float _normalMapStrength = 1.0f;
        float _parallaxOcclusionMapping = 0.0f;
        float _displacementMapping = 0.0f;
        float _refraction = 0.0f;
        float _transmission = 0.0f;

        MaterialTextureMap _textures[ecast(MaterialTextureSlot::Count)] = {};
    };
}
