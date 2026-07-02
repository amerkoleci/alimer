// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Module.h"
#include "Alimer/Assets/Asset.h"
#include "Alimer/Core/Containers.h"

namespace Alimer
{
	class AssetLoader;

	class ALIMER_API AssetManager final : public Module<AssetManager>
	{
	public:
		/// Constructor.
		AssetManager(const std::string& rootDirectory);

        /// Destructor.
        ~AssetManager() override;

		/// Return an asset by type and name. Load if not loaded yet. Return null if not found or if fails, unless SetReturnFailedResources(true) has been called. Can be called only from the main thread.
        SharedPtr<Asset> Load(const StringId32& type, std::string_view name, bool errorOnFailure = true);

        /// Store an external loaded/created asset.
        bool Store(Asset* asset);

        std::unique_ptr<Stream> OpenRead(std::string_view name, bool errorOnFailure = true);

		/// Template version of returning a resource by name.
		template <class T> SharedPtr<T> Load(std::string_view name, bool errorOnFailure = true)
		{
            static_assert((std::is_base_of<Asset, T>::value), "Specified type is not a valid Asset.");

			return StaticCast<T>(Load(T::GetTypeStatic(), name, errorOnFailure));
		}

        const std::string& GetRootDirectory() const { return rootDirectory; }

	private:
		std::string rootDirectory;
        UnorderedMap<StringId32, SharedPtr<Asset>> assets;
	};

	/** Provides easier access to Assets module. */
	ALIMER_API AssetManager& GetAssets();
}
