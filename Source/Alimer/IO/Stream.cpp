// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/IO/Stream.h"
#include "Alimer/Core/StringId.h"
#include "Alimer/Core/SerializeValue.h"
#include "Alimer/Math/Vector4.h"
#include "Alimer/Math/Quaternion.h"
//#include "Alimer/Math/Rect.h"
#include "Alimer/Assets/AssetRef.h"

using namespace Alimer;

const ObjectRef ObjectRef::Invalid(kInvalidObjectRefId);

void Stream::SetName(std::string_view newName)
{
    _name = newName;
}

bool Stream::ReadBool()
{
    return ReadUInt8() != 0;
}

uint8_t Stream::ReadUInt8()
{
    uint8_t ret;
    Read(&ret, sizeof ret);
    return ret;
}

uint16_t Stream::ReadUInt16()
{
    uint16_t ret;
    Read(&ret, sizeof ret);
    return ret;
}

uint32_t Stream::ReadUInt32()
{
    uint32_t ret;
    Read(&ret, sizeof ret);
    return ret;
}

uint64_t Stream::ReadUInt64()
{
    uint64_t ret;
    Read(&ret, sizeof ret);
    return ret;
}

int8_t Stream::ReadInt8()
{
    int8_t ret;
    Read(&ret, sizeof ret);
    return ret;
}

int16_t Stream::ReadInt16()
{
    int16_t ret;
    Read(&ret, sizeof ret);
    return ret;
}

int32_t Stream::ReadInt32()
{
    int32_t ret;
    Read(&ret, sizeof ret);
    return ret;
}

int64_t Stream::ReadInt64()
{
    int64_t ret;
    Read(&ret, sizeof ret);
    return ret;
}

float Stream::ReadFloat()
{
    float ret;
    Read(&ret, sizeof ret);
    return ret;
}

double Stream::ReadDouble()
{
    double ret;
    Read(&ret, sizeof ret);
    return ret;
}

UUID Stream::ReadUUID()
{
    UUID ret;
    Read(&ret, sizeof ret);
    return ret;
}

String Stream::ReadString()
{
    String ret;

    while (!IsEof())
    {
        char c = ReadInt8();
        if (!c)
            break;
        else
            ret += c;
    }

    return ret;
}

uint32_t Stream::ReadVLE()
{
    uint32_t ret;
    uint8_t byte;

    byte = ReadUInt8();
    ret = byte & 0x7f;
    if (byte < 0x80)
        return ret;

    byte = ReadUInt8();
    ret |= ((uint32_t)(byte & 0x7f)) << 7;
    if (byte < 0x80)
        return ret;

    byte = ReadUInt8();
    ret |= ((uint32_t)(byte & 0x7f)) << 14;
    if (byte < 0x80)
        return ret;

    byte = ReadUInt8();
    ret |= ((uint32_t)byte) << 21;
    return ret;
}

String Stream::ReadLine()
{
    String ret;

    while (!IsEof())
    {
        char c = ReadInt8();
        if (c == 10)
            break;

        if (c == 13)
        {
            // Peek next char to see if it's 10, and skip it too
            if (!IsEof())
            {
                char next = ReadInt8();
                if (next != 10)
                    Seek(GetPosition() - 1);
            }
            break;
        }

        ret += c;
    }

    return ret;
}

String Stream::ReadFileID()
{
    String result;
    result.resize(4);
    Read(result.data(), 4);
    return result;
}

StringId32 Stream::ReadStringId32()
{
    return StringId32(ReadUInt32());
}

Vector<uint8_t> Stream::ReadBytes(size_t count)
{
    if (!count)
        count = GetSize();

    Vector<uint8_t> result(count);
    Read(result.data(), result.size());
    return result;
}

void Stream::ReadBytes(Vector<uint8_t>& data, size_t count)
{
    if (!count)
        count = GetSize();

    data.resize(count);
    Read(data.data(), count);
}

Vector<uint8_t> Stream::ReadBuffer()
{
    Vector<uint8_t> result(ReadVLE());
    if (result.size())
    {
        Read(result.data(), result.size());
    }

    return result;
}

Vector2 Stream::ReadVector2()
{
    float data[2];
    Read(data, sizeof data);
    return Vector2(data);
}

Vector3 Stream::ReadVector3()
{
    float data[3];
    Read(data, sizeof data);
    return Vector3(data);
}

Vector4 Stream::ReadVector4()
{
    float data[4];
    Read(data, sizeof data);
    return Vector4(data);
}

Quaternion Stream::ReadQuaternion()
{
    float data[4];
    Read(data, sizeof data);
    return Quaternion(data);
}

#if TODO
Rect Stream::ReadRect()
{
    int32_t data[4];
    Read(data, sizeof data);
    return Rect(data[0], data[1], data[2], data[3]);
}

RectF Stream::ReadRectF()
{
    float data[4];
    Read(data, sizeof data);
    return RectF(data[0], data[1], data[2], data[3]);
}
#endif // TODO

ObjectRef Stream::ReadObjectRef()
{
    ObjectRef result(ReadUInt32());
    return result;
}

AssetRef Stream::ReadAssetRef()
{
    AssetRef ret;
    ret.FromBinary(*this);
    return ret;
}

AssetRefList Stream::ReadAssetRefList()
{
    AssetRefList ret;
    ret.FromBinary(*this);
    return ret;
}

SerializeValue Stream::ReadSerializeValue()
{
    SerializeValue ret;
    ret.FromBinary(*this);
    return ret;
}


void Stream::Write(const uint8_t& value)
{
    Write(&value, sizeof(uint8_t));
}

void Stream::Write(const uint16_t& value)
{
    Write(&value, sizeof(uint16_t));
}

void Stream::Write(const uint32_t& value)
{
    Write(&value, sizeof(uint32_t));
}

void Stream::Write(const uint64_t& value)
{
    Write(&value, sizeof(uint64_t));
}

void Stream::Write(const int8_t& value)
{
    Write(&value, sizeof(int8_t));
}

void Stream::Write(const int16_t& value)
{
    Write(&value, sizeof(int16_t));
}

void Stream::Write(const int32_t& value)
{
    Write(&value, sizeof(int32_t));
}

void Stream::Write(const int64_t& value)
{
    Write(&value, sizeof(int64_t));
}

void Stream::Write(const bool& value)
{
    Write((uint8_t)(value ? 1 : 0));
}

void Stream::Write(const float& value)
{
    Write(&value, sizeof(float));
}

void Stream::Write(const double& value)
{
    Write(&value, sizeof(double));
}

void Stream::Write(const char& value)
{
    Write(&value, sizeof(char));
}

void Stream::Write(const UUID& value)
{
    Write(&value, sizeof(UUID));
}

void Stream::Write(const char* value)
{
    return Write(StringView(value));
}

void Stream::Write(StringView value)
{
    const char* chars = value.data();
    // Count length to the first zero, because ReadString() does the same
    size_t length = CStringLength(chars);
    Write(chars, length + 1);
}

void Stream::Write(const String& value)
{
    const char* chars = value.data();
    // Count length to the first zero, because ReadString() does the same
    size_t length = CStringLength(chars);
    Write(chars, length + 1);
}

void Stream::Write(const StringId32& value)
{
    Write(value.GetHash());
}

void Stream::WriteFileID(const char* value, size_t length)
{
    const size_t minLength = Min<size_t>(length, 4U);

    Write(value, minLength);
    for (size_t i = length; i < 4; ++i)
    {
        Write((int8_t)' ');
    }
}

void Stream::WriteFileID(StringView value)
{
    size_t length = Min<size_t>(value.length(), 4U);

    Write(value.data(), length);
    for (size_t i = value.length(); i < 4; ++i)
    {
        Write((int8_t)' ');
    }
}

void Stream::WriteBuffer(const Vector<uint8_t>& value)
{
    size_t numBytes = value.size();

    WriteVLE(numBytes);
    if (numBytes)
    {
        Write(&value[0], numBytes);
    }
}

void Stream::WriteVLE(const size_t& value)
{
    uint8_t data[4];

    if (value < 0x80)
    {
        return Write((uint8_t)value);
    }
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

void Stream::WriteLine(StringView value)
{
    Write(value.data(), value.length());
    Write('\r');
    Write('\n');
}

void Stream::Write(const Vector2& value)
{
    Write(&value.x, sizeof(Vector2));
}

void Stream::Write(const Vector3& value)
{
    Write(&value.x, sizeof(Vector3));
}

void Stream::Write(const Vector4& value)
{
    Write(&value.x, sizeof(Vector4));
}

void Stream::Write(const Quaternion& value)
{
    Write(&value.x, sizeof(Quaternion));
}

void Stream::Write(const ObjectRef& value)
{
    Write(value.id);
}

void Stream::Write(const AssetRef& value)
{
    value.ToBinary(*this);
}

void Stream::Write(const AssetRefList& value)
{
    value.ToBinary(*this);
}

void Stream::Write(const SerializeValue& value)
{
    value.ToBinary(*this);
}

