// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Assert.h"
#include "Alimer/Core/String.h"
#include "Alimer/Core/Vector.h"
#include "Alimer/Core/UnorderedMap.h"

namespace Alimer
{
    class JsonValue;
    class Stream;

    /// JSON array type.
    using JsonArray = Vector<JsonValue>;
    /// JSON object type.
    using JsonObject = UnorderedMap<String, JsonValue>;

    /// JSON object iterator.
    using JsonObjectIterator = UnorderedMap<String, JsonValue>::iterator;
    /// Constant JSON object iterator.
    using ConstJsonObjectIterator = UnorderedMap<String, JsonValue>::const_iterator;

    /// JSON value types.
    enum class JsonValueType : uint8_t
    {
        Null = 0,
        Bool,
        Number,
        String,
        Array,
        Object,

        Count
    };

    /// JSON number type.
    enum class JSONNumberType : uint8_t
    {
        /// Not a number.
        NaN = 0,
        /// Integer.
        Int,
        /// Unsigned integer.
        Uint,
        // Int64.
        Int64,
        /// Uint64.
        Uint64,
        /// Double.
        Double
    };

    /// JSON value. Stores a boolean, string or number, or either an array or dictionary-like collection of nested values.
    class ALIMER_API JsonValue final
    {
        friend class JsonFile;

    public:
        /// Construct a null value.
        JsonValue() noexcept
        {

        }

        /// Construct a default value with defined type.
        explicit JsonValue(JsonValueType valueType, JSONNumberType numberType = JSONNumberType::NaN)
        {
            SetType(valueType, numberType);
        }

        /// Copy-construct.
        JsonValue(const JsonValue& value)
        {
            *this = value;
        }

        /// Construct from a boolean.
        JsonValue(bool value)
        {
            *this = value;
        }

        /// Construct from an integer number.
        JsonValue(int32_t value)
        {
            *this = value;
        }

        /// Construct from an unsigned integer number.
        JsonValue(uint32_t value)
        {
            *this = value;
        }
        /// Construct from an int64_t number.
        JsonValue(int64_t value)
        {
            *this = value;
        }
        /// Construct from an uint64_t number.
        JsonValue(uint64_t value)
        {
            *this = value;
        }
        /// Construct from a floating point number.
        JsonValue(float value)
        {
            *this = value;
        }
        /// Construct from a floating point number.
        JsonValue(double value)
        {
            *this = value;
        }
        /// Construct from a string.
        JsonValue(StringView value)
        {
            *this = value;
        }
        /// Construct from a string.
        JsonValue(const String& value)
        {
            *this = value;
        }
        /// Construct from a C string.
        JsonValue(const char* value)
        {
            *this = value;
        }
        /// Construct from a JSON object.
        JsonValue(const JsonArray& value)
        {
            *this = value;
        }
        /// Construct from a JSON object.
        JsonValue(const JsonObject& value)
        {
            *this = value;
        }
        /// Move-construct from another JSON value.
        JsonValue(JsonValue&& value) noexcept
        {
            *this = std::move(value);
        }
        /// Destructor.
        ~JsonValue()
        {
            SetType(JsonValueType::Null);
        }

        /// Assign a JSON value.
        JsonValue& operator = (const JsonValue& rhs);
        /// Move-assign from another JSON value.
        JsonValue& operator =(JsonValue&& rhs);
        /// Assign a boolean.
        JsonValue& operator = (bool rhs);
        /// Assign an integer number.
        JsonValue& operator = (int32_t rhs);
        /// Assign an unsigned integer number.
        JsonValue& operator = (uint32_t rhs);
        /// Assign an int64_t number.
        JsonValue& operator = (int64_t rhs);
        /// Assign an uint64_t number.
        JsonValue& operator = (uint64_t rhs);
        /// Assign a floating point number.
        JsonValue& operator = (float rhs);
        /// Assign a floating point number.
        JsonValue& operator = (double rhs);
        /// Assign a string_view.
        JsonValue& operator = (StringView value);
        /// Assign a string.
        JsonValue& operator = (const String& value);
        /// Assign a C string.
        JsonValue& operator = (const char* value);
        /// Assign a JSON array.
        JsonValue& operator = (const JsonArray& value);
        /// Assign a JSON object.
        JsonValue& operator = (const JsonObject& value);

        // JSON array functions
        /// Index as an array. Becomes an array if was not before.
        JsonValue& operator [] (size_t index);
        /// Const index as an array. Return a null value if not an array.
        const JsonValue& operator [] (size_t index) const;
        /// Push a value at the end. Becomes an array if was not before.
        void Push(JsonValue value);
        /// Remove the last value. No-op if not an array.
        void Pop();
        /// Insert a value at position. Becomes an array if was not before.
        void Insert(size_t index, JsonValue value);
        /// Remove indexed value. No-op if not an array.
        void Erase(size_t pos, size_t length = 1);
        /// Resize array. Becomes an array if was not before.
        void Resize(size_t newSize);
        /// Return last element JSON value.
        JsonValue& Back();
        /// Return last element JSON value.
        const JsonValue& Back() const;
        /// Return JSON value at index.
        const JsonValue& Get(size_t index) const;

        /// Index as an object. Becomes an object if was not before.
        JsonValue& operator [] (const String& key);
        /// Const index as an object. Return a null value if not an object.
        const JsonValue& operator [] (const String& key) const;
        /// Set JSON value with key.
        void Set(StringView key, JsonValue value);
        /// Set JSON value with key.
        void Set(const String& key, JsonValue value);
        /// Return JSON value with key.
        const JsonValue& Get(const String& key) const;

        /// Test for equality with another JSON value.
        bool operator == (const JsonValue& rhs) const;
        /// Test for inequality.
        bool operator != (const JsonValue& rhs) const { return !(*this == rhs); }

        /// Parse from a string. Return populated value on success, empty otherwise.
        static JsonValue Parse(StringView str, bool reportError = true);

        /// Parse from a string. Return true on success.
        bool FromString(StringView str, bool reportError = true);
        /// Parse from a binary stream.
        void FromBinary(Stream& source);
        /// Write to a string. Called recursively to write nested values.
        void ToString(String& dest, const String& indendation = "\t") const;
        /// Return as string.
        String ToString(const String& indendation = "\t") const;
        /// Serialize to a binary stream.
        void ToBinary(Stream& dest) const;
        
        /// Insert an associative value. Becomes an object if was not before.
        void Insert(const std::pair<String, JsonValue>& pair);
        
        /// Remove an associative value. No-op if not an object.
        bool Erase(const String& key);
        /// Clear array or object. No-op otherwise.
        void Clear();
        /// Set to an empty array.
        void SetEmptyArray();
        /// Set to an empty object.
        void SetEmptyObject();
        /// Set to null value.
        void SetNull();

        /// Return number of values for objects or arrays, or 0 otherwise.
        size_t Size() const;
        /// Return whether an object or array is empty. Return false if not an object or array.
        bool Empty() const;

        /// Return the value type.
        JsonValueType GetValueType() const;
        /// Return the number type.
        JSONNumberType GetNumberType() const;

        /// Return whether is null.
        bool IsNull() const { return GetValueType() == JsonValueType::Null; }
        /// Return whether is a bool.
        bool IsBool() const { return GetValueType() == JsonValueType::Bool; }
        /// Return whether is a number.
        bool IsNumber() const { return GetValueType() == JsonValueType::Number; }
        /// Return whether is a string.
        bool IsString() const { return GetValueType() == JsonValueType::String; }
        /// Return whether is an array.
        bool IsArray() const { return GetValueType() == JsonValueType::Array; }
        /// Return whether is an object.
        bool IsObject() const { return GetValueType() == JsonValueType::Object; }
        /// Return whether is an array or object.
        bool IsArrayOrObject() const { return IsArray() || IsObject(); }
        /// Return whether is a number and integer.
        bool IsInt()    const { return GetValueType() == JsonValueType::Number && GetNumberType() == JSONNumberType::Int; }
        /// Return whether is a number and unsigned integer.
        bool IsUint()   const { return GetValueType() == JsonValueType::Number && GetNumberType() == JSONNumberType::Uint; }
        /// Return whether is a number and integer.
        bool IsInt64()  const { return GetValueType() == JsonValueType::Number && GetNumberType() == JSONNumberType::Int64; }
        /// Return whether is a number and unsigned integer.
        bool IsUint64() const { return GetValueType() == JsonValueType::Number && GetNumberType() == JSONNumberType::Uint64; }
        /// Return whether is a double precision number.
        bool IsDouble() const { return GetValueType() == JsonValueType::Number && GetNumberType() == JSONNumberType::Double; }
        /// Return whether is a single precision number.
        bool IsFloat() const
        {
            return GetValueType() == JsonValueType::Number && GetNumberType() == JSONNumberType::Double;
        }

        /// Return value as a bool, or false on type mismatch.
        bool GetBool(bool defaultValue = false) const { return IsBool() ? _boolValue : defaultValue; }
        /// Return integer value.
        int32_t GetInt(int32_t defaultValue = 0) const { return IsNumber() ? (int32_t)_numberValue : defaultValue; }
        /// Return unsigned integer value.
        uint32_t GetUInt(uint32_t defaultValue = 0) const { return IsNumber() ? (uint32_t)_numberValue : defaultValue; }
        /// Return integer value.
        int64_t GetInt64(int64_t defaultValue = 0) const { return IsNumber() ? (int64_t)_numberValue : defaultValue; }
        /// Return integer value.
        uint64_t GetUInt64(uint64_t defaultValue = 0) const { return IsNumber() ? (uint64_t)_numberValue : defaultValue; }
        /// Return value as a number, or zero on type mismatch.
        double GetDouble(double defaultValue = 0.0) const { return IsNumber() ? _numberValue : defaultValue; }
        /// Return float value.
        float GetFloat(float defaultValue = 0.0f) const { return IsNumber() ? static_cast<float>(GetDouble()) : defaultValue; }

        size_t GetStringLength() const { ALIMER_ASSERT(IsString()); return _stringValue->length(); }
        /// Return value as a string, or empty string on type mismatch.
        const String& GetString(const String& defaultValue = kEmptyString) const { return IsString() ? *_stringValue : defaultValue; }
        /// Return C string value. Default to empty string literal.
        const char* GetCString(const char* defaultValue = "") const { return IsString() ? _stringValue->c_str() : defaultValue; }
        /// Return value as an array, or empty on type mismatch.
        const JsonArray& GetArray() const { return IsArray() ? *_arrayValue : emptyJSONArray; }
        /// Return value as an object, or empty on type mismatch.
        const JsonObject& GetObject() const { return IsObject() ? *_objectValue : emptyJSONObject; }
        /// Return whether has an associative value.
        bool Contains(const String& key) const;

        /// Empty (null) value.
        static const JsonValue EMPTY;
        /// Empty array.
        static const JsonArray emptyJSONArray;
        /// Empty object.
        static const JsonObject emptyJSONObject;

    protected:
        /// Assign a new type and perform the necessary dynamic allocation / deletion.
        void SetType(JsonValueType valueType, JSONNumberType numberType = JSONNumberType::NaN);

        /// Type.
        uint32_t _type{};

        union
        {
            /// Boolean value.
            bool _boolValue;
            /// Number value.
            double _numberValue;
            /// String value.
            String* _stringValue;
            /// Array value.
            JsonArray* _arrayValue;
            /// Object value.
            JsonObject* _objectValue;
        };

        friend ALIMER_API JsonObjectIterator begin(JsonValue& value);
        friend ALIMER_API ConstJsonObjectIterator begin(const JsonValue& value);
        friend ALIMER_API JsonObjectIterator end(JsonValue& value);
        friend ALIMER_API ConstJsonObjectIterator end(const JsonValue& value);
    };

    /// Return iterator to the beginning.
    ALIMER_API JsonObjectIterator begin(JsonValue& value);
    /// Return iterator to the beginning.
    ALIMER_API ConstJsonObjectIterator begin(const JsonValue& value);
    /// Return iterator to the end.
    ALIMER_API JsonObjectIterator end(JsonValue& value);
    /// Return iterator to the beginning.
    ALIMER_API ConstJsonObjectIterator end(const JsonValue& value);
}
