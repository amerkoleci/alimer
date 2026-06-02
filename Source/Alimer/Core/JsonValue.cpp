// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/JsonValue.h"
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
    // Convert rapidjson value to JSON value.
    static void FromRapidjsonValue(JsonValue& jsonValue, const rapidjson::Value& rapidjsonValue)
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
                    JsonValue& value = jsonValue[std::string(i->name.GetString())];
                    FromRapidjsonValue(value, i->value);
                }
            }
            break;

            default:
                break;
        }
    }

    static void ToRapidjsonValue(rapidjson::Value& rapidjsonValue, const JsonValue& jsonValue, rapidjson::MemoryPoolAllocator<>& allocator)
    {
        switch (jsonValue.GetValueType())
        {
            case JsonValueType::Null:
                rapidjsonValue.SetNull();
                break;

            case JsonValueType::Bool:
                rapidjsonValue.SetBool(jsonValue.GetBool());
                break;

            case JsonValueType::Number:
            {
                switch (jsonValue.GetNumberType())
                {
                    case JSONNumberType::Int:
                        rapidjsonValue.SetInt(jsonValue.GetInt());
                        break;

                    case JSONNumberType::Uint:
                        rapidjsonValue.SetUint(jsonValue.GetUInt());
                        break;

                    case JSONNumberType::Int64:
                        rapidjsonValue.SetInt64(jsonValue.GetInt64());
                        break;

                    case JSONNumberType::Uint64:
                        rapidjsonValue.SetUint64(jsonValue.GetUInt64());
                        break;

                    default:
                        rapidjsonValue.SetDouble(jsonValue.GetDouble());
                        break;
                }
            }
            break;

            case JsonValueType::String:
                rapidjsonValue.SetString(jsonValue.GetCString(), allocator);
                break;

            case JsonValueType::Array:
            {
                const JsonArray& jsonArray = jsonValue.GetArray();

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

            case JsonValueType::Object:
            {
                const JsonObject& jsonObject = jsonValue.GetObject();

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

const JsonValue JsonValue::EMPTY;
const JsonArray JsonValue::emptyJSONArray;
const JsonObject JsonValue::emptyJSONObject;

JsonValueType JsonValue::GetValueType() const
{
    return (JsonValueType)(_type >> 16u);
}

JSONNumberType JsonValue::GetNumberType() const
{
    return (JSONNumberType)(_type & 0xffffu);
}

JsonValue& JsonValue::operator = (const JsonValue& rhs)
{
    if (this == &rhs)
        return *this;

    SetType(rhs.GetValueType(), rhs.GetNumberType());

    switch (GetValueType())
    {
        case JsonValueType::Bool:
            _boolValue = rhs._boolValue;
            break;

        case JsonValueType::Number:
            _numberValue = rhs._numberValue;
            break;

        case JsonValueType::String:
            *_stringValue = *rhs._stringValue;
            break;

        case JsonValueType::Array:
            *_arrayValue = *rhs._arrayValue;
            break;

        case JsonValueType::Object:
            *_objectValue = *rhs._objectValue;
            break;

        default:
            break;
    }

    return *this;
}


JsonValue& JsonValue::operator=(JsonValue&& rhs)
{
    ALIMER_ASSERT(this != &rhs);

    SetType(rhs.GetValueType(), rhs.GetNumberType());

    switch (GetValueType())
    {
        case JsonValueType::Bool:
            _boolValue = rhs._boolValue;
            break;

        case JsonValueType::Number:
            _numberValue = rhs._numberValue;
            break;

        case JsonValueType::String:
            *_stringValue = std::move(*rhs._stringValue);
            break;

        case JsonValueType::Array:
            *_arrayValue = std::move(*rhs._arrayValue);
            break;

        case JsonValueType::Object:
            *_objectValue = std::move(*rhs._objectValue);

        default:
            break;
    }

    return *this;
}


JsonValue& JsonValue::operator = (bool rhs)
{
    SetType(JsonValueType::Bool);
    _boolValue = rhs;

    return *this;
}

JsonValue& JsonValue::operator = (int32_t rhs)
{
    SetType(JsonValueType::Number, JSONNumberType::Int);
    _numberValue = (double)rhs;

    return *this;
}

JsonValue& JsonValue::operator = (uint32_t rhs)
{
    SetType(JsonValueType::Number, JSONNumberType::Uint);
    _numberValue = (double)rhs;
    return *this;
}

JsonValue& JsonValue::operator = (int64_t rhs)
{
    SetType(JsonValueType::Number, JSONNumberType::Int64);
    _numberValue = (double)rhs;

    return *this;
}

JsonValue& JsonValue::operator = (uint64_t rhs)
{
    SetType(JsonValueType::Number, JSONNumberType::Uint64);
    _numberValue = (double)rhs;
    return *this;
}

JsonValue& JsonValue::operator = (float rhs)
{
    SetType(JsonValueType::Number, JSONNumberType::Double);
    _numberValue = (double)rhs;
    return *this;
}

JsonValue& JsonValue::operator = (double rhs)
{
    SetType(JsonValueType::Number, JSONNumberType::Double);
    _numberValue = rhs;
    return *this;
}

JsonValue& JsonValue::operator = (std::string_view value)
{
    SetType(JsonValueType::String);
    *_stringValue = value;

    return *this;
}

JsonValue& JsonValue::operator = (const std::string& value)
{
    SetType(JsonValueType::String);
    *_stringValue = value;

    return *this;
}

JsonValue& JsonValue::operator = (const char* value)
{
    SetType(JsonValueType::String);
    *_stringValue = value;

    return *this;
}

JsonValue& JsonValue::operator = (const JsonArray& value)
{
    SetType(JsonValueType::Array);
    *_arrayValue = value;

    return *this;
}

JsonValue& JsonValue::operator = (const JsonObject& value)
{
    SetType(JsonValueType::Object);
    *_objectValue = value;

    return *this;
}

JsonValue& JsonValue::operator [] (size_t index)
{
    SetType(JsonValueType::Array);

    return (*_arrayValue)[index];
}

const JsonValue& JsonValue::operator [] (size_t index) const
{
    if (GetValueType() != JsonValueType::Array)
        return EMPTY;

    return (*_arrayValue)[index];
}

void JsonValue::Push(JsonValue value)
{
    SetType(JsonValueType::Array);

    _arrayValue->push_back(std::move(value));
}

void JsonValue::Pop()
{
    if (GetValueType() != JsonValueType::Array)
        return;

    _arrayValue->pop_back();
}

void JsonValue::Insert(size_t pos, JsonValue value)
{
    if (GetValueType() != JsonValueType::Array)
        return;

    _arrayValue->insert(_arrayValue->begin() + pos, std::move(value));
}

void JsonValue::Erase(size_t pos, size_t length)
{
    if (GetValueType() != JsonValueType::Array)
        return;

    _arrayValue->erase(_arrayValue->begin() + pos, _arrayValue->begin() + pos + length);
}

void JsonValue::Resize(size_t newSize)
{
    SetType(JsonValueType::Array);

    _arrayValue->resize(newSize);
}

JsonValue& JsonValue::Back()
{
    SetType(JsonValueType::Array);

    return (*_arrayValue)[_arrayValue->size() - 1];
}

const JsonValue& JsonValue::Back() const
{
    if (GetValueType() != JsonValueType::Array)
        return EMPTY;

    return (*_arrayValue)[_arrayValue->size() - 1];
}

const JsonValue& JsonValue::Get(size_t index) const
{
    if (GetValueType() != JsonValueType::Array)
        return EMPTY;

    if (index < 0 || index >= _arrayValue->size())
        return EMPTY;

    return _arrayValue->at(index);
}

JsonValue& JsonValue::operator [] (const std::string& key)
{
    SetType(JsonValueType::Object);

    return (*_objectValue)[key];
}

const JsonValue& JsonValue::operator [] (const std::string& key) const
{
    if (GetValueType() != JsonValueType::Object)
        return EMPTY;

    auto it = _objectValue->find(key);
    return it != _objectValue->end() ? it->second : EMPTY;
}

void JsonValue::Set(std::string_view key, JsonValue value)
{
    // Convert to object type
    SetType(JsonValueType::Object);

    (*_objectValue)[key.data()] = std::move(value);
}

void JsonValue::Set(const std::string& key, JsonValue value)
{
    // Convert to object type
    SetType(JsonValueType::Object);

    (*_objectValue)[key] = std::move(value);
}

const JsonValue& JsonValue::Get(const std::string& key) const
{
    if (GetValueType() != JsonValueType::Object)
        return EMPTY;

    auto i = _objectValue->find(key);
    if (i == _objectValue->end())
        return EMPTY;

    return i->second;
}

bool JsonValue::operator == (const JsonValue& rhs) const
{
    // Value type without number type is checked. JSON does not make a distinction between number types. It is possible
    // that we serialized number (for example `1`) as unsigned integer. It will not necessarily be unserialized as same
    // number type. Number value equality check below will make sure numbers match anyway.
    if (GetValueType() != rhs.GetValueType())
        return false;

    switch (GetValueType())
    {
        case JsonValueType::Bool:
            return _boolValue == rhs._boolValue;

        case JsonValueType::Number:
            return _numberValue == rhs._numberValue;

        case JsonValueType::String:
            return *_stringValue == *rhs._stringValue;

        case JsonValueType::Array:
            return *_arrayValue == *rhs._arrayValue;

        case JsonValueType::Object:
            return *_objectValue == *rhs._objectValue;

        default:
            return true;
    }
}

JsonValue JsonValue::Parse(std::string_view str, bool reportError)
{
    JsonValue result;
    result.FromString(str, reportError);
    return result;
}

bool JsonValue::FromString(std::string_view str, bool reportError)
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

void JsonValue::FromBinary(Stream& source)
{
    JsonValueType newType = (JsonValueType)source.ReadUInt8();

    switch (newType)
    {
        case JsonValueType::Null:
            Clear();
            break;

        case JsonValueType::Bool:
            *this = source.ReadBool();
            break;

        case JsonValueType::Number:
            *this = source.ReadDouble();
            break;

        case JsonValueType::String:
            *this = source.ReadString();
            break;

        case JsonValueType::Array:
        {
            SetEmptyArray();
            size_t num = source.ReadVLE();
            for (size_t i = 0; i < num && !source.IsEof(); ++i)
                Push(source.ReadJsonValue());
        }
        break;

        case JsonValueType::Object:
        {
            SetEmptyObject();
            size_t num = source.ReadVLE();
            for (size_t i = 0; i < num && !source.IsEof(); ++i)
            {
                std::string key = source.ReadString();
                (*this)[key] = source.ReadJsonValue();
            }
        }
        break;

        default:
            break;
    }
}

void JsonValue::ToString(std::string& dest, const std::string& indendation) const
{
    rapidjson::Document document;
    ToRapidjsonValue(document, *this, document.GetAllocator());

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    writer.SetIndent(!indendation.empty() ? indendation.front() : '\0', (unsigned)indendation.length());

    document.Accept(writer);
    dest = buffer.GetString();
}

std::string JsonValue::ToString(const std::string& indendation) const
{
    std::string ret;
    ToString(ret, indendation);
    return ret;
}

void JsonValue::ToBinary(Stream& dest) const
{
    dest.Write((uint32_t)_type);

    switch (GetValueType())
    {
        case JsonValueType::Bool:
            dest.Write(_boolValue);
            break;

        case JsonValueType::Number:
            dest.Write(_numberValue);
            break;

        case JsonValueType::String:
            dest.Write(GetString());
            break;

        case JsonValueType::Array:
        {
            const JsonArray& array = GetArray();
            dest.WriteVLE(array.size());
            for (auto it = array.begin(); it != array.end(); ++it)
                it->ToBinary(dest);
        }
        break;

        case JsonValueType::Object:
        {
            const JsonObject& object = GetObject();
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

void JsonValue::Insert(const std::pair<std::string, JsonValue>& pair)
{
    SetType(JsonValueType::Object);

    _objectValue->insert(pair);
}

bool JsonValue::Erase(const std::string& key)
{
    if (GetValueType() != JsonValueType::Object)
        return false;

    return _objectValue->erase(key);
}

void JsonValue::Clear()
{
    if (GetValueType() == JsonValueType::Array)
        _arrayValue->clear();
    else if (GetValueType() == JsonValueType::Object)
        _objectValue->clear();
}

void JsonValue::SetEmptyArray()
{
    SetType(JsonValueType::Array);

    Clear();
}

void JsonValue::SetEmptyObject()
{
    SetType(JsonValueType::Object);

    Clear();
}

void JsonValue::SetNull()
{
    SetType(JsonValueType::Null);
}

size_t JsonValue::Size() const
{
    if (GetValueType() == JsonValueType::Array)
        return _arrayValue->size();
    else if (GetValueType() == JsonValueType::Object)
        return _objectValue->size();
    else
        return 0;
}

bool JsonValue::Empty() const
{
    if (GetValueType() == JsonValueType::Array)
        return _arrayValue->empty();
    else if (GetValueType() == JsonValueType::Object)
        return _objectValue->empty();
    else
        return false;
}

bool JsonValue::Contains(const std::string& key) const
{
    if (GetValueType() != JsonValueType::Object)
        return false;

    return _objectValue->find(key) != _objectValue->end();
}

void JsonValue::SetType(JsonValueType valueType, JSONNumberType numberType)
{
    uint32_t newType = (uint8_t)valueType << 16u | (uint8_t)numberType;
    if (newType == _type)
        return;

    switch (GetValueType())
    {
        case JsonValueType::String:
            delete _stringValue;
            break;

        case JsonValueType::Array:
            delete _arrayValue;
            break;

        case JsonValueType::Object:
            delete _objectValue;
            break;

        default:
            break;
    }

    _type = newType;

    switch (GetValueType())
    {
        case JsonValueType::String:
            _stringValue = new std::string();
            break;

        case JsonValueType::Array:
            _arrayValue = new JsonArray();
            break;

        case JsonValueType::Object:
            _objectValue = new JsonObject();
            break;

        default:
            break;
    }
}


JsonObjectIterator Alimer::begin(JsonValue& value)
{
    // Convert to object type.
    value.SetEmptyObject();

    return value._objectValue->begin();
}

ConstJsonObjectIterator Alimer::begin(const JsonValue& value)
{
    if (value.GetValueType() != JsonValueType::Object)
        return JsonValue::emptyJSONObject.begin();

    return value._objectValue->begin();
}

JsonObjectIterator Alimer::end(JsonValue& value)
{
    // Convert to object type.
    value.SetType(JsonValueType::Object);

    return value._objectValue->end();
}

ConstJsonObjectIterator Alimer::end(const JsonValue& value)
{
    if (value.GetValueType() != JsonValueType::Object)
        return JsonValue::emptyJSONObject.end();

    return value._objectValue->end();
}
