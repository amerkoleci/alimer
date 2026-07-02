// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/String.h"
#include "Alimer/Core/UUID.h"
#include "Alimer/IO/Types.h"

namespace Alimer
{
	class StringId32;
    struct Vector2;
    struct Vector3;
    struct Vector4;
    struct Quaternion;
    //struct Rect;
    //struct RectF;
    struct ObjectRef;
    struct AssetRef;
    struct AssetRefList;
    class SerializeValue;

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
        String ReadString();
		/// Read a variable-length encoded unsigned integer, which can use 29 bits maximum.
		uint32_t ReadVLE();
		/// Read a text line.
        String ReadLine();
		/// Read a 4-character file ID.
        String ReadFileID();
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
        //Rect ReadRect();
        /// Read a rect.
        //RectF ReadRectF();
        /// Read an object reference.
        ObjectRef ReadObjectRef();
        /// Read an asset reference.
        AssetRef ReadAssetRef();
        /// Read an asset list reference.
        AssetRefList ReadAssetRefList();
        /// Read a SerializeValue.
        SerializeValue ReadSerializeValue();

        /// Read enum.
        template<typename TEnum, typename = typename std::enable_if_t<std::is_enum_v<TEnum>>>
        TEnum ReadEnum()
        {
            TEnum data;
            Read(&data, sizeof(TEnum));
            return data;
        }

        /// Write an 8-bit unsigned integer.
        void Write(const uint8_t& value);
        /// Write a 16-bit unsigned integer.
        void Write(const uint16_t& value);
        /// Write a 32-bit unsigned integer.
        void Write(const uint32_t& value);
        /// Write a 64-bit unsigned integer.
        void Write(const uint64_t& value);
        /// Write an 8-bit signed integer.
        void Write(const int8_t& value);
        /// Write a 16-bit signed integer.
        void Write(const int16_t& value);
        /// Write a 32-bit signed integer.
        void Write(const int32_t& value);
        /// Write a 64-bit signed integer.
        void Write(const int64_t& value);
        /// Write a bool.
        void Write(const bool& value);
        /// Write a float.
        void Write(const float& value);
        /// Write a double.
        void Write(const double& value);
        /// Write a single UTF8 character.
        void Write(const char& value);
        /// Write a UUID.
        void Write(const UUID& value);
        /// Write a c-string.
        void Write(const char* value);
        /// Write a string.
        void Write(StringView value);
        /// Write a string.
        void Write(const String& value);
        /// Write a string id.
        void Write(const StringId32& value);
        /// Write a four-letter file ID. If the string is not long enough, spaces will be appended.
        void WriteFileID(const char* value, size_t length);
		/// Write a four-letter file ID. If the string is not long enough, spaces will be appended.
        void WriteFileID(StringView value);
		/// Write a byte buffer, with size encoded as VLE.
        void WriteBuffer(const Vector<uint8_t>& buffer);
		/// Write a variable-length encoded unsigned integer, which can use 29 bits maximum.
        void WriteVLE(const size_t& value);
        /// Write a text line. Char codes 13 & 10 will be automatically appended.
        void WriteLine(StringView value);
        /// Write a Vector2.
        void Write(const Vector2& value);
        /// Write a Vector3.
        void Write(const Vector3& value);
        /// Write a Vector4.
        void Write(const Vector4& value);
        /// Write a quaternion.
        void Write(const Quaternion& value);
        /// Write an object reference.
        void Write(const ObjectRef& value);
        /// Write an asset reference.
        void Write(const AssetRef& value);
        /// Write an asset list reference.
        void Write(const AssetRefList& value);
        /// Write a SerializeValue.
        void Write(const SerializeValue& value);
        /// Construct from enum.
        template<typename TEnum, typename = typename std::enable_if_t<std::is_enum_v<TEnum>>>
        void Write(const TEnum& value) noexcept
        {
            Write(&value, sizeof(TEnum));
        }

        /// Read a value, template version.
        template <typename T> T Read()
        {
            T result;
            Read(&result, sizeof(T));
            return result;
        }

        template<typename T> void Write(const T& data) const
        {
            Write(sizeof(T), &data);
        }

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
