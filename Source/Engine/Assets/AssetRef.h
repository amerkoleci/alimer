// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/StringId.h"
#include <vector>

namespace Alimer
{
	class Stream;

	/// Typed asset reference for serialization.
	struct ALIMER_API AssetRef
	{
		/// Resource type.
		StringId32 type;
		/// Resource name.
		std::string name;

		/// Constructor.
        AssetRef() = default;

        /// Construct from another AssetRef.
        AssetRef(const AssetRef& rhs) = default;

		/// Construct with type only and empty id.
		explicit AssetRef(StringId32 type_)
			: type(type_)
		{
		}

		/// Construct with type and resource name.
        AssetRef(StringId32 type, const std::string& name_)
			: type(type)
			, name(name_)
		{
		}

		/// Construct from a string.
        AssetRef(const std::string& str)
		{
			FromString(str);
		}

		/// Set from a string that contains the type and name separated by a semicolon. Return true on success.
		bool FromString(const std::string& str);
		/// Deserialize from a binary stream.
		void FromBinary(Stream& source);

		/// Return as a string.
        std::string ToString() const;
		/// Serialize to a binary stream.
		void ToBinary(Stream& dest) const;

		/// Test for equality with another reference.
		bool operator == (const AssetRef& rhs) const { return type == rhs.type && name == rhs.name; }
		/// Test for inequality with another reference.
		bool operator != (const AssetRef& rhs) const { return !(*this == rhs); }
	};
}
