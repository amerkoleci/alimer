// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Core/StringUtils.h"
#include "Core/Object.h"
#include "Assets/AssetRef.h"
#include "IO/Stream.h"

namespace Alimer
{
	bool AssetRef::FromString(const std::string& str)
	{
		std::vector<std::string> values = SplitNoEmpty(str, ";");
		if (values.size() == 2)
		{
			type = values[0];
			name = values[1];
			return true;
		}

		return false;
	}

	void AssetRef::FromBinary(Stream& source)
	{
		type = source.Read<StringId32>();
		name = source.Read<std::string>();
	}

	std::string AssetRef::ToString() const
	{
		return Object::GetTypeNameFromType(type) + ";" + name;
	}

	void AssetRef::ToBinary(Stream& dest) const
	{
		dest.Write(type);
		dest.Write(name);
	}
}
