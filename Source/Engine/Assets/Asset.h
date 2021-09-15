// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Object.h"

namespace Alimer
{
	class Stream;

	class ALIMER_API Asset : public Object
	{
        ALIMER_OBJECT(Asset, Object);

	public:
		/// Constructor.
        Asset() = default;

        /// Load asset synchronously. Call both BeginLoad() & EndLoad() and return true if both succeeded.
        bool Load(Stream& source);

        /// Save the asset to a stream. Return true on success.
        virtual bool Save(Stream& dest) const;

        /// Load resource from stream. May be called from a worker thread. Return true if successful.
        virtual bool BeginLoad(Stream& source);
        /// Finish resource loading. Always called from the main thread. Return true if successful.
        virtual bool EndLoad();

        /// Set name of the asset, usually the same as the file being loaded from.
        void SetName(const std::string& newName);

        /// Return name of the asset.
        const std::string& GetName() const { return name; }
        /// Return name id of the asset.
        const StringId32& GetNameId() const { return nameId; }

    private:
        /// Asset name.
        std::string name;
        /// Asset name id.
        StringId32 nameId;
	};
}
