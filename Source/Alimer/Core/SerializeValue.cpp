// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/SerializeValue.h"
#include "Alimer/IO/Stream.h"
#include "Alimer/Private/rapidjson_wrapper.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Alimer;
using namespace rapidjson;

namespace
{
    static_assert(sizeof(SerializeValueType) == sizeof(uint8_t), "Unexpected SerializeValueType size.");

    // Convert rapidjson value to JSON value.
    static void FromRapidjsonValue(SerializeValue& jsonValue, const rapidjson::Value& rapidjsonValue)
    {
        switch (rapidjsonValue.GetType())
        {
            case kNullType:
                // Reset to null type
                jsonValue.SetNull();
                break;

            case kFalseType:
                jsonValue = false;
                break;

            case kTrueType:
                jsonValue = true;
                break;

            case kNumberType:
                if (rapidjsonValue.IsInt())
                    jsonValue = rapidjsonValue.GetInt();
                else if (rapidjsonValue.IsUint())
                    jsonValue = rapidjsonValue.GetUint();
                else
                    jsonValue = rapidjsonValue.GetDouble();
                break;

            case kStringType:
                jsonValue = rapidjsonValue.GetString();
                break;

            case kArrayType:
            {
                jsonValue.Resize(rapidjsonValue.Size());
                for (rapidjson::SizeType i = 0; i < rapidjsonValue.Size(); ++i)
                {
                    FromRapidjsonValue(jsonValue[i], rapidjsonValue[i]);
                }
            }
            break;

            case kObjectType:
            {
                jsonValue.SetEmptyObject();
                for (rapidjson::Value::ConstMemberIterator i = rapidjsonValue.MemberBegin(); i != rapidjsonValue.MemberEnd(); ++i)
                {
                    SerializeValue& value = jsonValue[String(i->name.GetString())];
                    FromRapidjsonValue(value, i->value);
                }
            }
            break;

            default:
                break;
        }
    }

    static void ToRapidjsonValue(rapidjson::Value& rapidjsonValue, const SerializeValue& jsonValue, rapidjson::MemoryPoolAllocator<>& allocator)
    {
        switch (jsonValue.GetValueType())
        {
            case SerializeValueType::Null:
                rapidjsonValue.SetNull();
                break;

            case SerializeValueType::Bool:
                rapidjsonValue.SetBool(jsonValue.GetBool());
                break;

            case SerializeValueType::Number:
            {
                switch (jsonValue.GetNumberType())
                {
                    case SerializeValueNumberType::Int16:
                    case SerializeValueNumberType::Int32:
                        rapidjsonValue.SetInt(jsonValue.GetInt32());
                        break;

                    case SerializeValueNumberType::UInt16:
                    case SerializeValueNumberType::UInt32:
                        rapidjsonValue.SetUint(jsonValue.GetUInt32());
                        break;

                    case SerializeValueNumberType::Int64:
                        rapidjsonValue.SetInt64(jsonValue.GetInt64());
                        break;

                    case SerializeValueNumberType::UInt64:
                        rapidjsonValue.SetUint64(jsonValue.GetUInt64());
                        break;

                    default:
                        rapidjsonValue.SetDouble(jsonValue.GetDouble());
                        break;
                }
            }
            break;

            case SerializeValueType::String:
                rapidjsonValue.SetString(jsonValue.GetCString(), allocator);
                break;

            case SerializeValueType::Array:
            {
                const SerializeValueArray& jsonArray = jsonValue.GetArray();

                rapidjsonValue.SetArray();
                rapidjsonValue.Reserve((rapidjson::SizeType)jsonArray.size(), allocator);

                for (unsigned i = 0; i < jsonArray.size(); ++i)
                {
                    rapidjson::Value value;
                    ToRapidjsonValue(value, jsonArray[i], allocator);
                    rapidjsonValue.PushBack(value, allocator);
                }
            }
            break;

            case SerializeValueType::Object:
            {
                const SerializeValueObject& jsonObject = jsonValue.GetObject();

                rapidjsonValue.SetObject();
                for (auto i = jsonObject.begin(); i != jsonObject.end(); ++i)
                {
                    const char* name = i->first.c_str();
                    rapidjson::Value value;
                    ToRapidjsonValue(value, i->second, allocator);
                    rapidjsonValue.AddMember(StringRef(name), value, allocator);
                }
            }
            break;

            default:
                break;
        }
    }
}

const SerializeValue SerializeValue::Empty;
const SerializeValueArray SerializeValue::EmptyArray;
const SerializeValueObject SerializeValue::EmptyObject;

SerializeValueType SerializeValue::GetValueType() const
{
    return (SerializeValueType)(_type >> 16u);
}

SerializeValueNumberType SerializeValue::GetNumberType() const
{
    return (SerializeValueNumberType)(_type & 0xffffu);
}

SerializeValue& SerializeValue::operator = (const SerializeValue& rhs)
{
    if (this == &rhs)
        return *this;

    SetType(rhs.GetValueType(), rhs.GetNumberType());

    switch (GetValueType())
    {
        case SerializeValueType::Bool:
            _boolValue = rhs._boolValue;
            break;

        case SerializeValueType::Number:
            _numberValue = rhs._numberValue;
            break;

        case SerializeValueType::String:
            *_stringValue = *rhs._stringValue;
            break;

        case SerializeValueType::Array:
            *_arrayValue = *rhs._arrayValue;
            break;

        case SerializeValueType::Object:
            *_objectValue = *rhs._objectValue;
            break;

        default:
            break;
    }

    return *this;
}

SerializeValue& SerializeValue::operator=(SerializeValue&& rhs) 
{
    ALIMER_ASSERT(this != &rhs);

    SetType(rhs.GetValueType(), rhs.GetNumberType());

    switch (GetValueType())
    {
        case SerializeValueType::Bool:
            _boolValue = rhs._boolValue;
            break;

        case SerializeValueType::Number:
            _numberValue = rhs._numberValue;
            break;

        case SerializeValueType::String:
            *_stringValue = std::move(*rhs._stringValue);
            break;

        case SerializeValueType::Array:
            *_arrayValue = std::move(*rhs._arrayValue);
            break;

        case SerializeValueType::Object:
            *_objectValue = std::move(*rhs._objectValue);
            break;

        default:
            break;
    }

    return *this;
}


SerializeValue& SerializeValue::operator = (bool rhs)
{
    SetType(SerializeValueType::Bool);
    _boolValue = rhs;

    return *this;
}

SerializeValue& SerializeValue::operator = (int16_t rhs)
{
    SetType(SerializeValueType::Number, SerializeValueNumberType::Int16);
    _numberValue = (double)rhs;

    return *this;
}

SerializeValue& SerializeValue::operator = (uint16_t rhs)
{
    SetType(SerializeValueType::Number, SerializeValueNumberType::UInt16);
    _numberValue = (double)rhs;
    return *this;
}

SerializeValue& SerializeValue::operator = (int32_t rhs)
{
    SetType(SerializeValueType::Number, SerializeValueNumberType::Int32);
    _numberValue = (double)rhs;

    return *this;
}

SerializeValue& SerializeValue::operator = (uint32_t rhs)
{
    SetType(SerializeValueType::Number, SerializeValueNumberType::UInt32);
    _numberValue = (double)rhs;
    return *this;
}

SerializeValue& SerializeValue::operator = (int64_t rhs)
{
    SetType(SerializeValueType::Number, SerializeValueNumberType::Int64);
    _numberValue = (double)rhs;

    return *this;
}

SerializeValue& SerializeValue::operator = (uint64_t rhs)
{
    SetType(SerializeValueType::Number, SerializeValueNumberType::UInt64);
    _numberValue = (double)rhs;
    return *this;
}

SerializeValue& SerializeValue::operator = (float rhs)
{
    SetType(SerializeValueType::Number, SerializeValueNumberType::Double);
    _numberValue = (double)rhs;
    return *this;
}

SerializeValue& SerializeValue::operator = (double rhs)
{
    SetType(SerializeValueType::Number, SerializeValueNumberType::Double);
    _numberValue = rhs;
    return *this;
}

SerializeValue& SerializeValue::operator = (StringView value)
{
    SetType(SerializeValueType::String);
    *_stringValue = value;

    return *this;
}

SerializeValue& SerializeValue::operator = (const String& value)
{
    SetType(SerializeValueType::String);
    *_stringValue = value;

    return *this;
}

SerializeValue& SerializeValue::operator = (const char* value)
{
    SetType(SerializeValueType::String);
    *_stringValue = value;

    return *this;
}

SerializeValue& SerializeValue::operator = (const SerializeValueArray& value)
{
    SetType(SerializeValueType::Array);
    *_arrayValue = value;

    return *this;
}

SerializeValue& SerializeValue::operator = (const SerializeValueObject& value)
{
    SetType(SerializeValueType::Object);
    *_objectValue = value;

    return *this;
}

SerializeValue& SerializeValue::operator [] (size_t index)
{
    SetType(SerializeValueType::Array);

    return (*_arrayValue)[index];
}

const SerializeValue& SerializeValue::operator [] (size_t index) const
{
    if (GetValueType() != SerializeValueType::Array)
        return Empty;

    return (*_arrayValue)[index];
}

void SerializeValue::Push(SerializeValue value)
{
    SetType(SerializeValueType::Array);

    _arrayValue->push_back(std::move(value));
}

void SerializeValue::Pop()
{
    if (GetValueType() != SerializeValueType::Array)
        return;

    _arrayValue->pop_back();
}

void SerializeValue::Insert(size_t pos, SerializeValue value)
{
    if (GetValueType() != SerializeValueType::Array)
        return;

    _arrayValue->insert(_arrayValue->begin() + pos, std::move(value));
}

void SerializeValue::Erase(size_t pos, size_t length)
{
    if (GetValueType() != SerializeValueType::Array)
        return;

    _arrayValue->erase(_arrayValue->begin() + pos, _arrayValue->begin() + pos + length);
}

void SerializeValue::Resize(size_t newSize)
{
    SetType(SerializeValueType::Array);

    _arrayValue->resize(newSize);
}

SerializeValue& SerializeValue::Back()
{
    SetType(SerializeValueType::Array);

    return (*_arrayValue)[_arrayValue->size() - 1];
}

const SerializeValue& SerializeValue::Back() const
{
    if (GetValueType() != SerializeValueType::Array)
        return Empty;

    return (*_arrayValue)[_arrayValue->size() - 1];
}

const SerializeValue& SerializeValue::Get(size_t index) const
{
    if (GetValueType() != SerializeValueType::Array)
        return Empty;

    if (index < 0 || index >= _arrayValue->size())
        return Empty;

    return _arrayValue->at(index);
}

SerializeValue& SerializeValue::operator [] (const String& key)
{
    SetType(SerializeValueType::Object);

    return (*_objectValue)[key];
}

const SerializeValue& SerializeValue::operator [] (const String& key) const
{
    if (GetValueType() != SerializeValueType::Object)
        return Empty;

    auto it = _objectValue->find(key);
    return it != _objectValue->end() ? it->second : Empty;
}

void SerializeValue::Set(StringView key, SerializeValue value)
{
    // Convert to object type
    SetType(SerializeValueType::Object);

    (*_objectValue)[key.data()] = std::move(value);
}

void SerializeValue::Set(const String& key, SerializeValue value)
{
    // Convert to object type
    SetType(SerializeValueType::Object);

    (*_objectValue)[key] = std::move(value);
}

const SerializeValue& SerializeValue::Get(const String& key) const
{
    if (GetValueType() != SerializeValueType::Object)
        return Empty;

    auto i = _objectValue->find(key);
    if (i == _objectValue->end())
        return Empty;

    return i->second;
}

bool SerializeValue::operator == (const SerializeValue& rhs) const
{
    // Value type without number type is checked. JSON does not make a distinction between number types. It is possible
    // that we serialized number (for example `1`) as unsigned integer. It will not necessarily be unserialized as same
    // number type. Number value equality check below will make sure numbers match anyway.
    if (GetValueType() != rhs.GetValueType())
        return false;

    switch (GetValueType())
    {
        case SerializeValueType::Bool:
            return _boolValue == rhs._boolValue;

        case SerializeValueType::Number:
            return _numberValue == rhs._numberValue;

        case SerializeValueType::String:
            return *_stringValue == *rhs._stringValue;

        case SerializeValueType::Array:
            return *_arrayValue == *rhs._arrayValue;

        case SerializeValueType::Object:
            return *_objectValue == *rhs._objectValue;

        default:
            return true;
    }
}

SerializeValue SerializeValue::ParseJson(StringView str, bool reportError)
{
    SerializeValue result;
    result.FromJson(str, reportError);
    return result;
}

bool SerializeValue::FromJson(StringView str, bool reportError)
{
    Document document;
    document.Parse(str.data());
    if (document.HasParseError())
    {
        if (reportError)
        {
            LOGE("Failed to parse JSON data from string with error: {}", (uint32_t)document.GetParseError());
        }

        return false;
    }

    FromRapidjsonValue(*this, document);
    return true;
}

void SerializeValue::ToJson(String& dest, const String& indendation) const
{
    rapidjson::Document document;
    ToRapidjsonValue(document, *this, document.GetAllocator());

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    writer.SetIndent(!indendation.empty() ? indendation.front() : '\0', (unsigned)indendation.length());

    document.Accept(writer);
    dest = buffer.GetString();
}

String SerializeValue::ToJson(const String& indendation) const
{
    String ret;
    ToJson(ret, indendation);
    return ret;
}

void SerializeValue::FromBinary(Stream& source)
{
    SerializeValueType newType = (SerializeValueType)source.ReadUInt8();

    switch (newType)
    {
        case SerializeValueType::Null:
            Clear();
            break;

        case SerializeValueType::Bool:
            *this = source.ReadBool();
            break;

        case SerializeValueType::Number:
            *this = source.ReadDouble();
            break;

        case SerializeValueType::String:
            *this = source.ReadString();
            break;

        case SerializeValueType::Array:
        {
            SetEmptyArray();
            size_t num = source.ReadVLE();
            for (size_t i = 0; i < num && !source.IsEof(); ++i)
                Push(source.ReadSerializeValue());
        }
        break;

        case SerializeValueType::Object:
        {
            SetEmptyObject();
            size_t num = source.ReadVLE();
            for (size_t i = 0; i < num && !source.IsEof(); ++i)
            {
                String key = source.ReadString();
                (*this)[key] = source.ReadSerializeValue();
            }
        }
        break;

        default:
            break;
    }
}


void SerializeValue::ToBinary(Stream& dest) const
{
    dest.Write((uint32_t)_type);

    switch (GetValueType())
    {
        case SerializeValueType::Bool:
            dest.Write(_boolValue);
            break;

        case SerializeValueType::Number:
            dest.Write(_numberValue);
            break;

        case SerializeValueType::String:
            dest.Write(GetString());
            break;

        case SerializeValueType::Array:
        {
            const SerializeValueArray& array = GetArray();
            dest.WriteVLE(array.size());
            for (auto it = array.begin(); it != array.end(); ++it)
                it->ToBinary(dest);
        }
        break;

        case SerializeValueType::Object:
        {
            const SerializeValueObject& object = GetObject();
            dest.WriteVLE(object.size());
            for (auto it = object.begin(); it != object.end(); ++it)
            {
                dest.Write(it->first);
                it->second.ToBinary(dest);
            }
        }
        break;

        default:
            break;
    }
}

void SerializeValue::Insert(const std::pair<String, SerializeValue>& pair)
{
    SetType(SerializeValueType::Object);

    _objectValue->insert(pair);
}

bool SerializeValue::Erase(const String& key)
{
    if (GetValueType() != SerializeValueType::Object)
        return false;

    return _objectValue->erase(key);
}

void SerializeValue::Clear()
{
    if (GetValueType() == SerializeValueType::Array)
        _arrayValue->clear();
    else if (GetValueType() == SerializeValueType::Object)
        _objectValue->clear();
}

void SerializeValue::SetEmptyArray()
{
    SetType(SerializeValueType::Array);

    Clear();
}

void SerializeValue::SetEmptyObject()
{
    SetType(SerializeValueType::Object);

    Clear();
}

void SerializeValue::SetNull()
{
    SetType(SerializeValueType::Null);
}

size_t SerializeValue::Size() const
{
    if (GetValueType() == SerializeValueType::Array)
        return _arrayValue->size();
    else if (GetValueType() == SerializeValueType::Object)
        return _objectValue->size();
    else
        return 0;
}

bool SerializeValue::IsEmpty() const
{
    if (GetValueType() == SerializeValueType::Array)
        return _arrayValue->empty();
    else if (GetValueType() == SerializeValueType::Object)
        return _objectValue->empty();
    else
        return false;
}

bool SerializeValue::Contains(const String& key) const
{
    if (GetValueType() != SerializeValueType::Object)
        return false;

    return _objectValue->find(key) != _objectValue->end();
}

void SerializeValue::SetType(SerializeValueType valueType, SerializeValueNumberType numberType)
{
    uint32_t newType = (uint8_t)valueType << 16u | (uint8_t)numberType;
    if (newType == _type)
        return;

    switch (GetValueType())
    {
        case SerializeValueType::String:
            delete _stringValue;
            break;

        case SerializeValueType::Array:
            delete _arrayValue;
            break;

        case SerializeValueType::Object:
            delete _objectValue;
            break;

        default:
            break;
    }

    _type = newType;

    switch (GetValueType())
    {
        case SerializeValueType::String:
            _stringValue = new String();
            break;

        case SerializeValueType::Array:
            _arrayValue = new SerializeValueArray();
            break;

        case SerializeValueType::Object:
            _objectValue = new SerializeValueObject();
            break;

        default:
            break;
    }
}


SerializeValueObjectIterator Alimer::begin(SerializeValue& value)
{
    // Convert to object type.
    value.SetEmptyObject();

    return value._objectValue->begin();
}

ConstSerializeValueObjectIterator Alimer::begin(const SerializeValue& value)
{
    if (value.GetValueType() != SerializeValueType::Object)
        return SerializeValue::EmptyObject.begin();

    return value._objectValue->begin();
}

SerializeValueObjectIterator Alimer::end(SerializeValue& value)
{
    // Convert to object type.
    value.SetType(SerializeValueType::Object);

    return value._objectValue->end();
}

ConstSerializeValueObjectIterator Alimer::end(const SerializeValue& value)
{
    if (value.GetValueType() != SerializeValueType::Object)
        return SerializeValue::EmptyObject.end();

    return value._objectValue->end();
}
