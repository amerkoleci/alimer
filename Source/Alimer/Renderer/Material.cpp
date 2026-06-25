// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Renderer/Material.h"

using namespace Alimer;

void Material::Register()
{
    RegisterFactory<Material>();
}

Material::Material(StringView name)
    : Asset(name)
{
}

UnlitMaterial::UnlitMaterial(StringView name)
    : Material(name)
{
}

void UnlitMaterial::SetBaseColorTexture(RHITexture* texture, MaterialTextureUVChannel channel)
{
    _baseColorMap.resource = texture;
    _baseColorMap.channel = channel;
}

void UnlitMaterial::SetBaseColorTexture(const String& uri, MaterialTextureUVChannel channel)
{
    _baseColorMap.name = uri;
    _baseColorMap.resource.Reset();
    _baseColorMap.channel = channel;
}

/* PhysicallyBasedMaterial */
PhysicallyBasedMaterial::PhysicallyBasedMaterial(StringView name)
    : UnlitMaterial(name)
{
}

void PhysicallyBasedMaterial::SetTexture(MaterialTextureSlot slot, RHITexture* texture, MaterialTextureUVChannel channel)
{
    _textures[ecast(slot)].resource = texture;
    _textures[ecast(slot)].channel = channel;
}

void PhysicallyBasedMaterial::SetTexture(MaterialTextureSlot slot, const String& uri, MaterialTextureUVChannel channel)
{
    _textures[ecast(slot)].name = uri;
    _textures[ecast(slot)].resource.Reset();
    _textures[ecast(slot)].channel = channel;
}
