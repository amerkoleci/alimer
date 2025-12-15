// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

namespace Alimer
{
	class ALIMER_API UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);

        UUID(const UUID&) = default;
        UUID& operator=(const UUID&) = default;
        UUID(UUID&&) = default;
        UUID& operator=(UUID&&) = default;

		operator uint64_t() const { return value; }
	private:
        uint64_t value;
	};

    static_assert(sizeof(UUID) == sizeof(uint64_t), "Unexpected UUID size.");
}

namespace std
{
	template<>
	struct hash<Alimer::UUID>
	{
		std::size_t operator()(const Alimer::UUID& uuid) const noexcept
		{
			return hash<uint64_t>()(uuid);
		}
	};
}
