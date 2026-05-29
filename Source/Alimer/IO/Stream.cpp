// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/IO/Stream.h"
#include "Alimer/Core/StringId.h"
#include "Alimer/Math/Vector4.h"
#include "Alimer/Math/Quaternion.h"
//#include "Alimer/Math/Rect.h"
#include "Alimer/Assets/AssetRef.h"
#include "Alimer/Assets/JsonValue.h"

using namespace Alimer;

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

std::string Stream::ReadString()
{
    std::string ret;

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

std::string Stream::ReadLine()
{
    std::string ret;

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

std::string Stream::ReadFileID()
{
    std::string result;
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

AssetRef Stream::ReadAssetRef()
{
    AssetRef ret;
    ret.FromBinary(*this);
    return ret;
}

JsonValue Stream::ReadJsonValue()
{
    JsonValue ret;
    ret.FromBinary(*this);
    return ret;
}


bool Stream::Write(uint8_t value)
{
    return Write(&value, sizeof(uint8_t)) == sizeof(uint8_t);
}

bool Stream::Write(uint16_t value)
{
    return Write(&value, sizeof(uint16_t)) == sizeof(uint16_t);
}

bool Stream::Write(uint32_t value)
{
    return Write(&value, sizeof(uint32_t)) == sizeof(uint32_t);
}

bool Stream::Write(uint64_t value)
{
    return Write(&value, sizeof(uint64_t)) == sizeof(uint64_t);
}

bool Stream::Write(int8_t value)
{
    return Write(&value, sizeof(int8_t)) == sizeof(int8_t);
}

bool Stream::Write(int16_t value)
{
    return Write(&value, sizeof(int16_t)) == sizeof(int16_t);
}

bool Stream::Write(int32_t value)
{
    return Write(&value, sizeof(int32_t)) == sizeof(int32_t);
}

bool Stream::Write(int64_t value)
{
    return Write(&value, sizeof(int64_t)) == sizeof(int64_t);
}

bool Stream::Write(bool value)
{
    return Write((uint8_t)(value ? 1 : 0)) == 1;
}

bool Stream::Write(float value)
{
    return Write(&value, sizeof(float)) == sizeof(float);
}

bool Stream::Write(double value)
{
    return Write(&value, sizeof(double)) == sizeof(double);
}

bool Stream::Write(const char* value)
{
    // Count length to the first zero, because ReadString() does the same
    const size_t length = CStringLength(value);

    return Write(value, length + 1) == length + 1;
}

bool Stream::Write(const std::string& value)
{
    const char* chars = value.c_str();
    // Count length to the first zero, because ReadString() does the same
    size_t length = CStringLength(chars);
    return Write(chars, length + 1) == length + 1;
}

bool Stream::Write(const StringId32& value)
{
    return Write(value.GetHash());
}

bool Stream::WriteFileID(const char* value, size_t length)
{
    bool success = true;
    const size_t minLength = Min<size_t>(length, 4U);

    success &= Write(value, minLength) == minLength;
    for (size_t i = length; i < 4; ++i)
    {
        success &= Write((int8_t)' ');
    }

    return success;
}

bool Stream::WriteFileID(const std::string& value)
{
    bool success = true;
    size_t length = Min<size_t>(value.length(), 4U);

    success &= Write(value.c_str(), length) == length;
    for (size_t i = value.length(); i < 4; ++i)
    {
        success &= Write((int8_t)' ');
    }

    return success;
}

bool Stream::WriteBuffer(const Vector<uint8_t>& value)
{
    size_t numBytes = value.size();

    bool success = WriteVLE(numBytes);
    if (numBytes)
    {
        success &= Write(&value[0], numBytes) == numBytes;
    }

    return success;
}

bool Stream::WriteVLE(size_t value)
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
        return Write(data, 2);
    }
    else if (value < 0x200000)
    {
        data[0] = (uint8_t)value | 0x80;
        data[1] = (uint8_t)((value >> 7) | 0x80);
        data[2] = (uint8_t)(value >> 14);
        return Write(data, 3);
    }
    else
    {
        data[0] = (uint8_t)value | 0x80;
        data[1] = (uint8_t)((value >> 7) | 0x80);
        data[2] = (uint8_t)((value >> 14) | 0x80);
        data[3] = (uint8_t)(value >> 21);
        return Write(data, 4);
    }
}

bool Stream::WriteLine(const std::string& value)
{
    bool success = Write(value.c_str(), value.length()) == value.length();
    success &= Write((uint8_t)'\r');
    success &= Write((uint8_t)'\n');
    return success;
}

bool Stream::WriteVector2(const Vector2& value)
{
    return Write(&value.x, sizeof(Vector2)) == sizeof(Vector2);
}

bool Stream::WriteVector3(const Vector3& value)
{
    return Write(&value.x, sizeof(Vector3)) == sizeof(Vector3);
}

bool Stream::WriteVector4(const Vector4& value)
{
    return Write(&value.x, sizeof(Vector4)) == sizeof(Vector4);
}

bool Stream::WriteQuaternion(const Quaternion& value)
{
    return Write(&value.x, sizeof(Quaternion)) == sizeof(Quaternion);
}

bool Stream::WriteAssetRef(const AssetRef& value)
{
    return value.ToBinary(*this);
}

void Stream::Write(const JsonValue& value)
{
    value.ToBinary(*this);
}

