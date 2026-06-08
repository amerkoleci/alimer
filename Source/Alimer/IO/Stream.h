// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/String.h"
#include "Alimer/Core/Vector.h"
#include "Alimer/Core/UUID.h"
#include "Alimer/IO/Types.h"

namespace Alimer
{
	class StringId32;
    struct Vector2;
    struct Vector3;
    struct Vector4;
    struct Quaternion;
    struct Rect;
    struct RectF;
    struct AssetRef;
    struct AssetRefList;
    class JsonValue;

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
		void SetName(std::string_view newName);
		/// Read a boolean.
		bool ReadBool();
        /// Read an 8-bit unsigned integer.
        uint8_t ReadUInt8();
        /// Read a 16-bit unsigned integer.
        uint16_t ReadUInt16();
        /// Read a 32-bit unsigned integer.
        uint32_t ReadUInt32();
        /// Read a 64-bit unsigned integer.
        uint64_t ReadUInt64();
        /// Read an 8-bit integer.
        int8_t ReadInt8();
        /// Read a 16-bit integer.
        int16_t ReadInt16();
        /// Read a 32-bit integer.
        int32_t ReadInt32();
        /// Read a 64-bit integer.
        int64_t ReadInt64();
        /// Read a float.
        float ReadFloat();
        /// Read a double.
        double ReadDouble();
        /// Read a UUID.
        UUID ReadUUID();
		/// Read a null-terminated string.
		std::string ReadString();
		/// Read a variable-length encoded unsigned integer, which can use 29 bits maximum.
		uint32_t ReadVLE();
		/// Read a text line.
        std::string ReadLine();
		/// Read a 4-character file ID.
        std::string ReadFileID();
        /// Read a 32-bit StringId32.
        StringId32 ReadStringId32();
		/// Read a vector bytes
        Vector<uint8_t> ReadBytes(size_t count = 0);
        /// Read a vector bytes
        void ReadBytes(Vector<uint8_t>& data, size_t count = 0);
		/// Read a byte buffer, with size prepended as a VLE value.
        Vector<uint8_t> ReadBuffer();
        /// Read a Vector2.
        Vector2 ReadVector2();
        /// Read a Vector3.
        Vector3 ReadVector3();
        /// Read a Vector4.
        Vector4 ReadVector4();
        /// Read a quaternion.
        Quaternion ReadQuaternion();
        /// Read a rect.
        Rect ReadRect();
        /// Read a rect.
        RectF ReadRectF();
        /// Read an asset reference.
        AssetRef ReadAssetRef();
        /// Read a JsonValue.
        JsonValue ReadJsonValue();

        /// Read a value, template version.
        template <typename T> void Read(T& data)
        {
            Read(&data, sizeof(T));
        }

        template<typename T> void Write(const T& data) const
        {
            Write(sizeof(T), &data);
        }

        /// Write an 8-bit unsigned integer.
        bool Write(uint8_t value);
        /// Write a 16-bit unsigned integer.
        bool Write(uint16_t value);
        /// Write a 32-bit unsigned integer.
        bool Write(uint32_t value);
        /// Write a 64-bit unsigned integer.
        bool Write(uint64_t value);
        /// Write an 8-bit signed integer.
        bool Write(int8_t value);
        /// Write a 16-bit signed integer.
        bool Write(int16_t value);
        /// Write a 32-bit signed integer.
        bool Write(int32_t value);
        /// Write a 64-bit signed integer.
        bool Write(int64_t value);
        /// Write a bool.
        bool Write(bool value);
        /// Write a float.
        bool Write(float value);
        /// Write a double.
        bool Write(double value);
        /// Write a c-string.
        bool Write(const char* value);
        /// Write a string.
        bool Write(const std::string& value);
        /// Write a string id.
        bool Write(const StringId32& value);
        /// Write a four-letter file ID. If the string is not long enough, spaces will be appended.
        bool WriteFileID(const char* value, size_t length);
		/// Write a four-letter file ID. If the string is not long enough, spaces will be appended.
		bool WriteFileID(const std::string& value);
		/// Write a byte buffer, with size encoded as VLE.
		bool WriteBuffer(const Vector<uint8_t>& buffer);
		/// Write a variable-length encoded unsigned integer, which can use 29 bits maximum.
        bool WriteVLE(size_t value);
		/// Write a text line. Char codes 13 & 10 will be automatically appended.
		bool WriteLine(const std::string& value);
        /// Write a Vector2.
        bool WriteVector2(const Vector2& value);
        /// Write a Vector3.
        bool WriteVector3(const Vector3& value);
        /// Write a Vector4.
        bool WriteVector4(const Vector4& value);
        /// Write a quaternion.
        bool WriteQuaternion(const Quaternion& value);
        /// Write an asset reference.
        bool WriteAssetRef(const AssetRef& value);
        /// Write a JSON value.
        void Write(const JsonValue& value);

		/// Return the stream name.
		const std::string& GetName() const { return _name; }
		/// Returns the position of the stream
        virtual size_t GetPosition() const = 0;
		/// Return the size of the stream.
        virtual size_t GetSize() const = 0;
		/// Return whether the end of stream has been reached.
		bool IsEof() const { return GetPosition() >= GetSize(); }

	protected:
		/// Stream name.
        std::string _name;
	};
}
