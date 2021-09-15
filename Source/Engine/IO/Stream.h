// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Types.h"
#include <vector>

namespace Alimer
{
	class StringId32;
	struct AssetRef;

	/// Abstract stream for reading and writing.
	class ALIMER_API Stream
	{
	public:
		/// Default-construct with zero size.
		Stream() = default;
		/// Destructor.
		virtual ~Stream() = default;

		/// Read bytes from the stream. Return number of bytes actually read.
		virtual size_t Read(void* buffer, size_t length) = 0;
		/// Seeks the position of the stream
		virtual size_t Seek(size_t position) = 0;
		/// Write bytes to the stream. Return number of bytes actually written.
		virtual size_t Write(const void* data, size_t size) = 0;
		/// Return whether read operations are allowed.
		virtual bool CanRead() const = 0;
		/// Return whether write operations are allowed.
		virtual bool CanWrite() const = 0;

		/// Change the stream name.
		void SetName(const std::string& newName);
		/// Read a boolean.
		bool ReadBoolean();
		// reads a string of a given length, or until a null terminator if -1
		std::string ReadString(int length = -1);
		/// Read a variable-length encoded unsigned integer, which can use 29 bits maximum.
		uint32_t ReadVLE();
		/// Read a text line.
        std::string ReadLine();
		/// Read a 4-character file ID.
        std::string ReadFileID();
		/// Read a vector bytes
		std::vector<uint8_t> ReadBytes(size_t count = 0);
		/// Read a byte buffer, with size prepended as a VLE value.
		std::vector<uint8_t> ReadBuffer();
		/// Write a four-letter file ID. If the string is not long enough, spaces will be appended.
		void WriteFileID(const std::string& value);
		/// Write a byte buffer, with size encoded as VLE.
		void WriteBuffer(const std::vector<uint8_t>& buffer);
		/// Write a variable-length encoded unsigned integer, which can use 29 bits maximum.
		void WriteVLE(size_t value);
		/// Write a text line. Char codes 13 & 10 will be automatically appended.
		void WriteLine(const std::string& value);

		/// Write a value, template version.
		template <class T> void Write(const T& value) { Write(&value, sizeof value); }

		/// Read a value, template version.
		template <class T> T Read()
		{
			T ret;
			Read(&ret, sizeof ret);
			return ret;
		}

		/// Return the stream name.
		const std::string& GetName() const { return name; }
		/// Returns the position of the stream
        virtual size_t Position() const = 0;
		/// Return the length of the stream.
        virtual size_t Length() const = 0;
		/// Return whether the end of stream has been reached.
		bool IsEof() const { return Position() >= Length(); }

	protected:
		/// Stream name.
        std::string name;
	};

	template<> ALIMER_API bool Stream::Read();
	template<> ALIMER_API std::string Stream::Read();
	template<> ALIMER_API StringId32 Stream::Read();
	template<> ALIMER_API AssetRef Stream::Read();
	//template<> ALIMER_API JSONValue Stream::Read();
	template<> ALIMER_API void Stream::Write(const bool& value);
	template<> ALIMER_API void Stream::Write(const std::string& value);
	template<> ALIMER_API void Stream::Write(const StringId32& value);
	template<> ALIMER_API void Stream::Write(const AssetRef& value);
	//template<> ALIMER_API void Stream::Write(const JSONValue& value);

}
