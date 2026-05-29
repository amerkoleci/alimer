// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/IO/FileSystem.h"
#include "Alimer/Assets/AssetManager.h"
#include "Alimer/Assets/JsonFile.h"
#include "Alimer/Assets/Image.h"
//#include "Alimer/Assets/Texture.h"
//#include "Alimer/Renderer/Mesh.h"
//#include "Alimer/Renderer/Material.h"
//#include "Alimer/Renderer/Font.h"

using namespace Alimer;

AssetManager::AssetManager(const std::string& rootDirectory)
    : rootDirectory{ rootDirectory }
{
    JsonFile::Register();
    Image::Register();
    //Texture::Register();
    //Shader::Register();
    //Mesh::Register();
    //Material::Register();
    //Font::Register();
}

AssetManager::~AssetManager()
{
    // Material::defaultMaterial.Reset();
}

SharedPtr<Asset> AssetManager::Load(const StringId32& type, std::string_view name, bool errorOnFailure)
{
    // Check if already loaded/created.
    StringId32 nameHash(name);
    auto it = assets.find(nameHash);
    if (it != assets.end())
    {
        return it->second;
    }

    // Open stream first.
    std::unique_ptr<Stream> stream = OpenRead(name, errorOnFailure);
    if (!stream)
    {
        return nullptr;
    }

    SharedPtr<Asset> asset = CreateObject<Asset>(type);
    if (asset == nullptr)
    {
        if (errorOnFailure)
            LOGE("Could not load unknown resource type {}", type.ToString());
        else
            LOGW("Could not load unknown resource type {}", type.ToString());

        return nullptr;
    }

    asset->SetName(stream->GetName());
    if (!asset->Load(*stream.get()))
    {
        return nullptr;
    }

    assets[nameHash] = asset;
    return asset;
}

std::unique_ptr<Stream> AssetManager::OpenRead(std::string_view name, bool errorOnFailure)
{
    // Open stream first.
    std::string path = Path::Combine(rootDirectory, name);
    if (!File::Exists(path))
    {
        if (errorOnFailure)
            LOGE("Asset file '{}' doesn't exists", path);
        else
            LOGW("Asset file '{}' doesn't exists", path);
        return {};
    }

    // TODO: VirtualStream, CompressStream, Asset Package loading

    // Open stream
    return std::make_unique<FileStream>(path, FileMode::Read);
}

bool AssetManager::Store(Asset* asset)
{
    auto it = assets.find(asset->GetNameId());
    if (it != assets.end())
    {
        return false;
    }

    assets[asset->GetNameId()] = asset;
    return true;
}

AssetManager& Alimer::GetAssets()
{
    return AssetManager::Instance();
}
