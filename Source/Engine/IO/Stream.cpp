// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "IO/Stream.h"
#include "Math/MathHelper.h"
//#include "IO/JSONValue.h"
#include "Assets/AssetRef.h"

namespace Alimer
{
	void Stream::SetName(const std::string& newName)
	{
		name = newName;
	}

	bool Stream::ReadBoolean()
	{
		return Read<uint8_t>() != 0;
	}

	std::string Stream::ReadString(int length)
	{
		if (length >= 0)
		{
			std::string str;
			str.resize(length);
			Read(str.data(), length);
			str[length] = '\0';
			return str;
		}
		else
		{
			std::string str;
			char next;
			while (Read(&next, 1) && next != '\0')
			{
				str += next;
			}

			return str;
		}
	}

	uint32_t Stream::ReadVLE()
	{
		uint32_t ret;
		uint8_t byte;

		byte = Read<uint8_t>();
		ret = byte & 0x7f;
		if (byte < 0x80)
			return ret;

		byte = Read<uint8_t>();
		ret |= ((uint32_t)(byte & 0x7f)) << 7;
		if (byte < 0x80)
			return ret;

		byte = Read<uint8_t>();
		ret |= ((uint32_t)(byte & 0x7f)) << 14;
		if (byte < 0x80)
			return ret;

		byte = Read<uint8_t>();
		ret |= ((uint32_t)byte) << 21;
		return ret;
	}

    std::string Stream::ReadLine()
	{
        std::string ret;

		while (!IsEof())
		{
			char c = Read<char>();
			if (c == 10)
				break;
			if (c == 13)
			{
				// Peek next char to see if it's 10, and skip it too
				if (!IsEof())
				{
					char next = Read<char>();
					if (next != 10)
						Seek(Position() - 1);
				}
				break;
			}

			ret += c;
		}

		return ret;
	}

    std::string Stream::ReadFileID()
	{
        std::string result;
		result.resize(4);
		Read(result.data(), 4);
		return result;
	}

	std::vector<uint8_t> Stream::ReadBytes(size_t count)
	{
		if (!count)
			count = Length();

		std::vector<uint8_t> result(count);
		Read(result.data(), result.size());
		return result;
	}

	std::vector<uint8_t> Stream::ReadBuffer()
	{
		std::vector<uint8_t> result(ReadVLE());
		if (result.size())
		{
			Read(result.data(), result.size());
		}

		return result;
	}

	template<> bool Stream::Read<bool>()
	{
		return ReadBoolean();
	}

	template<> std::string Stream::Read<std::string>()
	{
		return ReadString();
	}

	template<> StringId32 Stream::Read<StringId32>()
	{
		return StringId32(Read<uint32_t>());
	}

	template<> AssetRef Stream::Read<AssetRef>()
	{
        AssetRef ret;
		ret.FromBinary(*this);
		return ret;
	}

	void Stream::WriteFileID(const std::string& value)
	{
		Write(value.c_str(), Min(value.length(), (size_t)4));
		for (size_t i = value.length(); i < 4; ++i)
		{
			Write(' ');
		}
	}

	void Stream::WriteBuffer(const std::vector<uint8_t>& value)
	{
		size_t numBytes = value.size();

		WriteVLE(numBytes);
		if (numBytes)
		{
			Write(&value[0], numBytes);
		}
	}

	void Stream::WriteVLE(size_t value)
	{
		uint8_t data[4];

		if (value < 0x80)
			Write((uint8_t)value);
		else if (value < 0x4000)
		{
			data[0] = (uint8_t)value | 0x80;
			data[1] = (uint8_t)(value >> 7);
			Write(data, 2);
		}
		else if (value < 0x200000)
		{
			data[0] = (uint8_t)value | 0x80;
			data[1] = (uint8_t)((value >> 7) | 0x80);
			data[2] = (uint8_t)(value >> 14);
			Write(data, 3);
		}
		else
		{
			data[0] = (uint8_t)value | 0x80;
			data[1] = (uint8_t)((value >> 7) | 0x80);
			data[2] = (uint8_t)((value >> 14) | 0x80);
			data[3] = (uint8_t)(value >> 21);
			Write(data, 4);
		}
	}

	void Stream::WriteLine(const std::string& value)
	{
		Write(value.c_str(), value.length());
		Write('\r');
		Write('\n');
	}

	template<> void Stream::Write<bool>(const bool& value)
	{
		Write<unsigned char>(value ? 1 : 0);
	}

	template<> void Stream::Write<std::string>(const std::string& value)
	{
		// Write content and null terminator
		Write(value.c_str(), value.length() + 1);
	}

	template<> void Stream::Write<StringId32>(const StringId32& value)
	{
		Write(value.Value());
	}

	template<> void Stream::Write<AssetRef>(const AssetRef& value)
	{
		value.ToBinary(*this);
	}

	/*template<> void Stream::Write<JSONValue>(const JSONValue& value)
	{
		value.ToBinary(*this);
	}*/
}
