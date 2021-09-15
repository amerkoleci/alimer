// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/StringUtils.h"

namespace Alimer
{
	/// 32-bit hash value for a string.
	class ALIMER_API StringId32
	{
	public:
		/// Construct with zero value.
		StringId32() noexcept
			: value(0)
		{
		}

		/// Copy-construct from another hash.
		StringId32(const StringId32& rhs) noexcept = default;

		/// Construct with an initial value.
		explicit StringId32(uint32_t value_) noexcept
			: value(value_)
		{
		}

		/// Construct from a C string.
		StringId32(const char* str) noexcept;
		/// Construct from a string.
		StringId32(const std::string& str) noexcept;

		/// Assign from another hash.
		StringId32& operator=(const StringId32& rhs) noexcept = default;

		/// Add a hash.
		StringId32 operator+(const StringId32& rhs) const
		{
			StringId32 ret;
			ret.value = value + rhs.value;
			return ret;
		}

		/// Add-assign a hash.
		StringId32& operator+=(const StringId32& rhs)
		{
			value += rhs.value;
			return *this;
		}

		/// Test for equality with another hash.
		bool operator==(const StringId32& rhs) const { return value == rhs.value; }

		/// Test for inequality with another hash.
		bool operator!=(const StringId32& rhs) const { return value != rhs.value; }

		/// Test if less than another hash.
		bool operator<(const StringId32& rhs) const { return value < rhs.value; }

		/// Test if greater than another hash.
		bool operator>(const StringId32& rhs) const { return value > rhs.value; }

		/// Return true if nonzero hash value.
		explicit operator bool() const { return value != 0; }

		/// Return hash value.
		uint32_t Value() const { return value; }

		/// Return as string.
		std::string ToString() const;

		/// Zero hash.
		static const StringId32 Zero;

	private:
		/// Hash value.
		uint32_t value;
	};

	static_assert(sizeof(StringId32) == sizeof(uint32_t), "Unexpected StringHash size.");
}

namespace std
{
	template <> class hash<Alimer::StringId32>
	{
	public:
		size_t operator()(const Alimer::StringId32& value) const { return value.Value(); }
	};
}
