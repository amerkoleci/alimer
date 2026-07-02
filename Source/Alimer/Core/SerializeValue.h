// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Assert.h"
#include "Alimer/Core/String.h"
#include "Alimer/Core/Vector.h"
#include "Alimer/Core/UnorderedMap.h"

namespace Alimer
{
    class SerializeValue;
    class Stream;

    /// SerializeValue array type.
    using SerializeValueArray = Vector<SerializeValue>;
    /// SerializeValue object type.
    using SerializeValueObject = UnorderedMap<String, SerializeValue>;

    /// SerializeValue object iterator.
    using SerializeValueObjectIterator = UnorderedMap<String, SerializeValue>::iterator;
    /// Constant SerializeValue object iterator.
    using ConstSerializeValueObjectIterator = UnorderedMap<String, SerializeValue>::const_iterator;

    /// SerializeValue types.
    enum class SerializeValueType : uint8_t
    {
        Null = 0,
        Bool,
        Number,
        String,
        Array,
        Object,

        Count
    };

    /// SerializeValue number type.
    enum class SerializeValueNumberType : uint8_t
    {
        /// Not a number.
        NaN = 0,
        /// 16-bit signed integer.
        Int16,
        /// 16-bit unsigned integer.
        UInt16,
        /// 32-bit signed integer.
        Int32,
        /// 32-bit unsigned integer.
        UInt32,
        /// 64-bit signed integer.
        Int64,
        /// 64-bit unsigned integer.
        UInt64,
        /// Double.
        Double
    };

    /// Serialize value. Stores a boolean, string or number, or either an array or dictionary-like collection of nested values.
    /// Can be loaded/saved from/to Json or other formats.
    class ALIMER_API SerializeValue final
    {
    public:
        /// Construct a null value.
        SerializeValue() noexcept
        {

        }

        /// Construct a default value with defined type.
        explicit SerializeValue(SerializeValueType valueType, SerializeValueNumberType numberType = SerializeValueNumberType::NaN)
        {
            SetType(valueType, numberType);
        }

        /// Copy-construct.
        SerializeValue(const SerializeValue& value)
        {
            *this = value;
        }

        /// Construct from a boolean.
        SerializeValue(bool value)
        {
            *this = value;
        }

        /// Construct from an integer number.
        SerializeValue(int16_t value)
        {
            *this = value;
        }

        /// Construct from an unsigned integer number.
        SerializeValue(uint16_t value)
        {
            *this = value;
        }

        /// Construct from an integer number.
        SerializeValue(int32_t value)
        {
            *this = value;
        }

        /// Construct from an unsigned integer number.
        SerializeValue(uint32_t value)
        {
            *this = value;
        }

        /// Construct from an int64_t number.
        SerializeValue(int64_t value)
        {
            *this = value;
        }

        /// Construct from an uint64_t number.
        SerializeValue(uint64_t value)
        {
            *this = value;
        }

        /// Construct from a floating point number.
        SerializeValue(float value)
        {
            *this = value;
        }

        /// Construct from a floating point number.
        SerializeValue(double value)
        {
            *this = value;
        }

        /// Construct from a string.
        SerializeValue(StringView value)
        {
            *this = value;
        }
        /// Construct from a string.
        SerializeValue(const String& value)
        {
            *this = value;
        }

        /// Construct from a C string.
        SerializeValue(const char* value)
        {
            *this = value;
        }

        /// Construct from a serialize value array.
        SerializeValue(const SerializeValueArray& value)
        {
            *this = value;
        }

        /// Construct from a serialize value object.
        SerializeValue(const SerializeValueObject& value)
        {
            *this = value;
        }

        /// Move-construct from another serialize value.
        SerializeValue(SerializeValue&& value) noexcept
        {
            *this = std::move(value);
        }

        /// Destructor.
        ~SerializeValue()
        {
            SetType(SerializeValueType::Null);
        }

        /// Assign a serialize value.
        SerializeValue& operator = (const SerializeValue& rhs);
        /// Move-assign from another serialize value.
        SerializeValue& operator =(SerializeValue&& rhs);
        /// Assign a boolean.
        SerializeValue& operator = (bool rhs);
        /// Assign an 16-bit signed integer number.
        SerializeValue& operator = (int16_t rhs);
        /// Assign a 16-bit unsigned integer number.
        SerializeValue& operator = (uint16_t rhs);
        /// Assign a 32-bit signed integer number.
        SerializeValue& operator = (int32_t rhs);
        /// Assign a 32-bit unsigned integer number.
        SerializeValue& operator = (uint32_t rhs);
        /// Assign a 64-bit signed integer number.
        SerializeValue& operator = (int64_t rhs);
        /// Assign a 64-bit unsigned integer number.
        SerializeValue& operator = (uint64_t rhs);
        /// Assign a floating point number.
        SerializeValue& operator = (float rhs);
        /// Assign a floating point number.
        SerializeValue& operator = (double rhs);
        /// Assign a string_view.
        SerializeValue& operator = (StringView value);
        /// Assign a string.
        SerializeValue& operator = (const String& value);
        /// Assign a C string.
        SerializeValue& operator = (const char* value);
        /// Assign a serialize value array.
        SerializeValue& operator = (const SerializeValueArray& value);
        /// Assign a serialize value object.
        SerializeValue& operator = (const SerializeValueObject& value);

        // Serialize value array functions
        /// Index as an array. Becomes an array if was not before.
        SerializeValue& operator [] (size_t index);
        /// Const index as an array. Return a null value if not an array.
        const SerializeValue& operator [] (size_t index) const;
        /// Push a value at the end. Becomes an array if was not before.
        void Push(SerializeValue value);
        /// Remove the last value. No-op if not an array.
        void Pop();
        /// Insert a value at position. Becomes an array if was not before.
        void Insert(size_t index, SerializeValue value);
        /// Remove indexed value. No-op if not an array.
        void Erase(size_t pos, size_t length = 1);
        /// Resize array. Becomes an array if was not before.
        void Resize(size_t newSize);
        /// Return last element serialize value.
        SerializeValue& Back();
        /// Return last element serialize value.
        const SerializeValue& Back() const;
        /// Return serialize value at index.
        const SerializeValue& Get(size_t index) const;

        /// Index as an object. Becomes an object if was not before.
        SerializeValue& operator [] (const String& key);
        /// Const index as an object. Return a null value if not an object.
        const SerializeValue& operator [] (const String& key) const;
        /// Set serialize value with key.
        void Set(StringView key, SerializeValue value);
        /// Set serialize value with key.
        void Set(const String& key, SerializeValue value);
        /// Return serialize value with key.
        const SerializeValue& Get(const String& key) const;

        /// Test for equality with another serialize value.
        bool operator == (const SerializeValue& rhs) const;
        /// Test for inequality.
        bool operator != (const SerializeValue& rhs) const { return !(*this == rhs); }

        /// Parse from a json string. Return populated value on success, empty otherwise.
        [[nodiscard]] static SerializeValue ParseJson(StringView str, bool reportError = true);

        /// Parse from a json string. Return true on success.
        bool FromJson(StringView str, bool reportError = true);
        /// Write to a json string. Called recursively to write nested values.
        void ToJson(String& dest, const String& indendation = "\t") const;
        /// Return as json string.
        String ToJson(const String& indendation = "\t") const;
        /// Parse from a binary stream.
        void FromBinary(Stream& source);
        /// Serialize to a binary stream.
        void ToBinary(Stream& dest) const;

        /// Insert an associative value. Becomes an object if was not before.
        void Insert(const std::pair<String, SerializeValue>& pair);

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
        bool IsEmpty() const;

        /// Return the value type.
        SerializeValueType GetValueType() const;
        /// Return the number type.
        SerializeValueNumberType GetNumberType() const;

        /// Return whether is null.
        bool IsNull() const { return GetValueType() == SerializeValueType::Null; }
        /// Return whether is a bool.
        bool IsBool() const { return GetValueType() == SerializeValueType::Bool; }
        /// Return whether is a number.
        bool IsNumber() const { return GetValueType() == SerializeValueType::Number; }
        /// Return whether is a string.
        bool IsString() const { return GetValueType() == SerializeValueType::String; }
        /// Return whether is an array.
        bool IsArray() const { return GetValueType() == SerializeValueType::Array; }
        /// Return whether is an object.
        bool IsObject() const { return GetValueType() == SerializeValueType::Object; }
        /// Return whether is an array or object.
        bool IsArrayOrObject() const { return IsArray() || IsObject(); }
        /// Return whether is a number and 16-bit signed integer.
        bool IsInt16()    const { return GetValueType() == SerializeValueType::Number && GetNumberType() == SerializeValueNumberType::Int16; }
        /// Return whether is a number and 16-bit unsigned integer.
        bool IsUInt16()   const { return GetValueType() == SerializeValueType::Number && GetNumberType() == SerializeValueNumberType::UInt16; }
        /// Return whether is a number and 32-bit signed integer.
        bool IsInt32()    const { return GetValueType() == SerializeValueType::Number && GetNumberType() == SerializeValueNumberType::Int32; }
        /// Return whether is a number and 32-bit unsigned integer.
        bool IsUInt32()   const { return GetValueType() == SerializeValueType::Number && GetNumberType() == SerializeValueNumberType::UInt32; }
        /// Return whether is a number and 64-bit signed integer.
        bool IsInt64()  const { return GetValueType() == SerializeValueType::Number && GetNumberType() == SerializeValueNumberType::Int64; }
        /// Return whether is a number and 64-bit unsigned integer.
        bool IsUInt64() const { return GetValueType() == SerializeValueType::Number && GetNumberType() == SerializeValueNumberType::UInt64; }
        /// Return whether is a double precision number.
        bool IsDouble() const { return GetValueType() == SerializeValueType::Number && GetNumberType() == SerializeValueNumberType::Double; }
        /// Return whether is a single precision number.
        bool IsFloat() const { return GetValueType() == SerializeValueType::Number && GetNumberType() == SerializeValueNumberType::Double; }

        /// Return value as a bool, or false on type mismatch.
        bool GetBool(bool defaultValue = false) const { return IsBool() ? _boolValue : defaultValue; }
        /// Return 16-bit signed integer value.
        int16_t GetInt16(int16_t defaultValue = 0) const { return IsNumber() ? (int16_t)_numberValue : defaultValue; }
        /// Return 16-bit unsigned integer value.
        uint16_t GetUInt16(uint16_t defaultValue = 0) const { return IsNumber() ? (uint16_t)_numberValue : defaultValue; }
        /// Return 32-bit signed integer value.
        int32_t GetInt32(int32_t defaultValue = 0) const { return IsNumber() ? (int32_t)_numberValue : defaultValue; }
        /// Return 32-bit unsigned integer value.
        uint32_t GetUInt32(uint32_t defaultValue = 0) const { return IsNumber() ? (uint32_t)_numberValue : defaultValue; }
        /// Return 64-bit signed integer value.
        int64_t GetInt64(int64_t defaultValue = 0) const { return IsNumber() ? (int64_t)_numberValue : defaultValue; }
        /// Return 64-bit unsigned integer value.
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
        const SerializeValueArray& GetArray() const { return IsArray() ? *_arrayValue : EmptyArray; }
        /// Return value as an object, or empty on type mismatch.
        const SerializeValueObject& GetObject() const { return IsObject() ? *_objectValue : EmptyObject; }
        /// Return whether has an associative value.
        bool Contains(const String& key) const;

        /// Empty (null) value.
        static const SerializeValue Empty;
        /// Empty array.
        static const SerializeValueArray EmptyArray;
        /// Empty object.
        static const SerializeValueObject EmptyObject;

    protected:
        /// Assign a new type and perform the necessary dynamic allocation / deletion.
        void SetType(SerializeValueType valueType, SerializeValueNumberType numberType = SerializeValueNumberType::NaN);

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
            SerializeValueArray* _arrayValue;
            /// Object value.
            SerializeValueObject* _objectValue;
        };

        friend ALIMER_API SerializeValueObjectIterator begin(SerializeValue& value);
        friend ALIMER_API ConstSerializeValueObjectIterator begin(const SerializeValue& value);
        friend ALIMER_API SerializeValueObjectIterator end(SerializeValue& value);
        friend ALIMER_API ConstSerializeValueObjectIterator end(const SerializeValue& value);
    };

    /// Return iterator to the beginning.
    ALIMER_API SerializeValueObjectIterator begin(SerializeValue& value);
    /// Return iterator to the beginning.
    ALIMER_API ConstSerializeValueObjectIterator begin(const SerializeValue& value);
    /// Return iterator to the end.
    ALIMER_API SerializeValueObjectIterator end(SerializeValue& value);
    /// Return iterator to the beginning.
    ALIMER_API ConstSerializeValueObjectIterator end(const SerializeValue& value);
}
