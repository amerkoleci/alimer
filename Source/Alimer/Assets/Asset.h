// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Object.h"
#include "Alimer/Core/UUID.h"
#include "Alimer/Assets/AssetRef.h"

namespace Alimer
{
	class Stream;

	class ALIMER_API Asset : public Object
	{
        ALIMER_OBJECT(Asset, Object);

	public:
		/// Constructor.
        Asset(std::string_view name = kEmptyStringView);

        /// Load asset synchronously. Call both BeginLoad() & EndLoad() and return true if both succeeded.
        bool Load(Stream& source);

        /// Save the asset to a stream. Return true on success.
        virtual bool Save(Stream& dest) const;

        /// Load resource from stream. May be called from a worker thread. Return true if successful.
        virtual bool BeginLoad(Stream& source);
        /// Finish resource loading. Always called from the main thread. Return true if successful.
        virtual bool EndLoad();

        /// Set name of the asset, usually the same as the file being loaded from.
        virtual void SetName(std::string_view name);

        /// Return name of the asset.
        [[nodiscard]] const std::string& GetName() const { return name; }
        /// Return name id of the asset.
        [[nodiscard]] const StringId32& GetNameId() const { return nameId; }
        /// Return id of the asset.
        [[nodiscard]] UUID GetID() const { return id; }
        /// Return memory use in bytes, possibly approximate.
        [[nodiscard]] uint32_t GetMemoryUse() const { return memoryUse; }

    protected:
        /// Set memory use in bytes, possibly approximate.
        void SetMemoryUse(uint32_t size);

        UUID id;
        /// Asset name.
        std::string name;
        /// Asset name id.
        StringId32 nameId;
        /// Memory use in bytes.
        uint32_t memoryUse{};
	};

    /// Return name from an asset.
    inline const std::string& AssetName(Asset* asset)
    {
        return asset ? asset->GetName() : kEmptyString;
    }

    /// Return type from a resource pointer, or default type if null.
    inline StringId32 AssetType(Asset* asset, StringId32 defaultType)
    {
        return asset ? asset->GetType() : defaultType;
    }

    /// Make a asset ref from an asset.
    inline AssetRef MakeAssetRef(Asset* asset, StringId32 defaultType)
    {
        return AssetRef(AssetType(asset, defaultType), AssetName(asset));
    }
}
