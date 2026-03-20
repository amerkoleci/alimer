// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/RefCounted.h"
#include "Alimer/Math/Quaternion.h"
#include "Alimer/Math/Color.h"
#include "Alimer/Math/Matrix3x2.h"
#include "Alimer/Math/Matrix4x4.h"
//#include "Alimer/Math/Rect.h"
//#include "Alimer/Math/BoundingBox.h"
//#include "Alimer/Math/BoundingSphere.h"
//#include "Alimer/Math/Plane.h"
//#include "Alimer/Math/Ray.h"
#include "Alimer/Assets/AssetRef.h"

namespace Alimer
{
    enum class VariantType : uint8_t
    {
        None = 0,

        Bool,
        Int8,
        UInt8,
        Int16,
        UInt16,
        Int32,
        UInt32,
        Int64,
        UInt64,
        Float,
        Double,
        Enum,
        String,
        ByteVector,
        StringVector,

        Pointer,
        RefPtr,
        AssetRef,
        AssetRefList,

        Vector2,
        Vector3,
        Vector4,
        Quaternion,
        Color,

#if TODO
        Rect,
        RectF,
        Int2,
        Int3,
        Int4,
        UInt2,
        UInt3,
        UInt4,
        Double2,
        Double3,
        Double4,

        
        BoundingBox,
        BoundingSphere,
        Plane,
        Ray,
        Matrix3x2,
        Matrix3x3,
        Matrix3x4,
        Matrix4x3,
        Matrix4x4,
        
#endif // TODO


        Count
    };

    /// Vector of bytes.
    using ByteVector = Vector<uint8_t>;

    /// Return variant size in bytes from type. This is not the same as size of Variant class instance, this is a size of corresponding type.
    ALIMER_API uint32_t GetVariantTypeSize(VariantType type);

    /// Return variant typename from type. 
    ALIMER_API std::string GetVariantTypeName(VariantType type);

    /// Return variant type from type name. 
    ALIMER_API VariantType GetVariantTypeFromName(std::string_view typeName);

    /// Return variant type from type.
    template <typename T> VariantType GetVariantType();

    // Return variant type from concrete types
    template <> inline VariantType GetVariantType<bool>() { return VariantType::Bool; }
    template <> inline VariantType GetVariantType<int8_t>() { return VariantType::Int8; }
    template <> inline VariantType GetVariantType<uint8_t>() { return VariantType::UInt8; }
    template <> inline VariantType GetVariantType<int16_t>() { return VariantType::Int16; }
    template <> inline VariantType GetVariantType<uint16_t>() { return VariantType::UInt16; }
    template <> inline VariantType GetVariantType<int32_t>() { return VariantType::Int32; }
    template <> inline VariantType GetVariantType<uint32_t>() { return VariantType::UInt32; }
    template <> inline VariantType GetVariantType<int64_t>() { return VariantType::Int64; }
    template <> inline VariantType GetVariantType<uint64_t>() { return VariantType::UInt64; }
    template <> inline VariantType GetVariantType<float>() { return VariantType::Float; }
    template <> inline VariantType GetVariantType<double>() { return VariantType::Double; }
    template <> inline VariantType GetVariantType<String>() { return VariantType::String; }
    template <> inline VariantType GetVariantType<ByteVector>() { return VariantType::ByteVector; }
    template <> inline VariantType GetVariantType<StringVector>() { return VariantType::StringVector; }
    template <> inline VariantType GetVariantType<StringId32>() { return VariantType::UInt32; }
    //template <> inline VariantType GetVariantType<UUID>() { return VariantType::UInt64; }
    template <> inline VariantType GetVariantType<void*>() { return VariantType::Pointer; }
    template <> inline VariantType GetVariantType<RefCounted*>() { return VariantType::RefPtr; }
    template <> inline VariantType GetVariantType<AssetRef>() { return VariantType::AssetRef; }
    template <> inline VariantType GetVariantType<AssetRefList>() { return VariantType::AssetRefList; }

    template <> inline VariantType GetVariantType<Vector2>() { return VariantType::Vector2; }
    template <> inline VariantType GetVariantType<Vector3>() { return VariantType::Vector3; }
    template <> inline VariantType GetVariantType<Vector4>() { return VariantType::Vector4; }
    template <> inline VariantType GetVariantType<Quaternion>() { return VariantType::Quaternion; }
    template <> inline VariantType GetVariantType<Color>() { return VariantType::Color; }
#if TODO
    template <> inline VariantType GetVariantType<Rect>() { return VariantType::Rect; }
    template <> inline VariantType GetVariantType<RectF>() { return VariantType::RectF; }

    template <> inline VariantType GetVariantType<Int2>() { return VariantType::Int2; }
    template <> inline VariantType GetVariantType<Int3>() { return VariantType::Int3; }
    template <> inline VariantType GetVariantType<Int4>() { return VariantType::Int4; }

    template <> inline VariantType GetVariantType<UInt2>() { return VariantType::UInt2; }
    template <> inline VariantType GetVariantType<UInt3>() { return VariantType::UInt3; }
    template <> inline VariantType GetVariantType<UInt4>() { return VariantType::UInt4; }

    template <> inline VariantType GetVariantType<Double2>() { return VariantType::Double2; }
    template <> inline VariantType GetVariantType<Double3>() { return VariantType::Double3; }
    template <> inline VariantType GetVariantType<Double4>() { return VariantType::Double4; }

    template <> inline VariantType GetVariantType<BoundingBox>() { return VariantType::BoundingBox; }
    template <> inline VariantType GetVariantType<BoundingSphere>() { return VariantType::BoundingSphere; }
    template <> inline VariantType GetVariantType<Plane>() { return VariantType::Plane; }
    template <> inline VariantType GetVariantType<Ray>() { return VariantType::Ray; }
    template <> inline VariantType GetVariantType<Matrix3x2>() { return VariantType::Matrix3x2; }
    template <> inline VariantType GetVariantType<Matrix3x3>() { return VariantType::Matrix3x3; }
    template <> inline VariantType GetVariantType<Matrix3x4>() { return VariantType::Matrix3x4; }
    template <> inline VariantType GetVariantType<Matrix4x3>() { return VariantType::Matrix4x3; }
    template <> inline VariantType GetVariantType<Matrix4x4>() { return VariantType::Matrix4x4; }
#endif // TODO


    /// Size of variant value. 16 bytes on 32-bit platform, 32 bytes on 64-bit platform.
    static const uint32_t kVariantValueSize = sizeof(void*) * 4;

    /// Union for the possible variant values. Objects exceeding the kVariantValueSize are allocated on the heap.
    union VariantValue
    {
        uint8_t storage[kVariantValueSize]; // std::string
        bool boolValue;
        int8_t int8;
        uint8_t uint8;
        int16_t int16;
        uint16_t uint16;
        int32_t int32;
        uint32_t uint32;
        int64_t int64;
        uint64_t uint64;
        float floatValue;
        double doubleVal;
        String string; // 40 bytes size
        StringVector stringVector;
        ByteVector byteVector;
        void* pointer;
        WeakPtr<RefCounted> weakPtr;
        AssetRef assetRef;
        AssetRefList assetRefList;

        Vector2 vector2;
        Vector3 vector3;
        Vector4 vector4;
        Quaternion quaternion;
        Color color;
#if TODO
        Rect rect;
        RectF rectF;
        Int2 int2;
        Int3 int3;
        Int4 int4;
        UInt2 uint2;
        UInt3 uint3;
        UInt4 uint4;
        Double2 double2;
        Double3 double3;
        Double4 double4;
#endif // TODO

        /// Construct uninitialized.
        VariantValue() {}      // NOLINT(modernize-use-equals-default)
        /// Non-copyable.
        VariantValue(const VariantValue& value) = delete;
        /// Destruct.
        ~VariantValue() {}     // NOLINT(modernize-use-equals-default)
    };

    class ALIMER_API Variant final
    {
    public:
        /// Construct empty.
        Variant() = default;

        /// Construct from a bool.
        Variant(bool value) noexcept
        {
            *this = value;
        }

        /// Construct from int8 value.
        Variant(int8_t value) noexcept
        {
            *this = value;
        }

        /// Construct from uint8 value.
        Variant(uint8_t value) noexcept
        {
            *this = value;
        }

        /// Construct from int16 value.
        Variant(int16_t value) noexcept
        {
            *this = value;
        }

        /// Construct from uint16 value.
        Variant(uint16_t value) noexcept
        {
            *this = value;
        }

        /// Construct from int32 value.
        Variant(int32_t value) noexcept
        {
            *this = value;
        }

        /// Construct from uint32 value.
        Variant(uint32_t value) noexcept
        {
            *this = value;
        }

        /// Construct from int64 value.
        Variant(int64_t value) noexcept
        {
            *this = value;
        }

        /// Construct from uint64 value.
        Variant(uint64_t value) noexcept
        {
            *this = value;
        }

        /// Construct from a float value.
        Variant(float value) noexcept
        {
            *this = value;
        }

        /// Construct from a double.
        Variant(double value) noexcept
        {
            *this = value;
        }

        /// Construct from a string.
        Variant(const String& value) noexcept
        {
            *this = value;
        }

        /// Construct from a C string.
        Variant(const char* value) noexcept
        {
            *this = value;
        }

        /// Construct from enum.
        template<typename TEnum, typename = typename std::enable_if_t<std::is_enum_v<TEnum>>>
        Variant(const TEnum value) noexcept
        {
            *this = value;
        }

        /// Construct from a byte vector.
        Variant(const ByteVector& value)  // NOLINT(google-explicit-constructor)
        {
            *this = value;
        }

        /// Construct from a string vector.
        Variant(const StringVector& value)  // NOLINT(google-explicit-constructor)
        {
            *this = value;
        }

        /// Construct from a string hash (convert to integer).
        Variant(const StringId32& value)    // NOLINT(google-explicit-constructor)
        {
            *this = value.GetHash();
        }

        /// Construct from a pointer.
        Variant(void* value) noexcept
        {
            *this = value;
        }

        /// Construct from a RefCounted pointer. The object will be stored internally in a WeakPtr so that its expiration can be detected safely.
        Variant(RefCounted* value) noexcept
        {
            *this = value;
        }

        /// Construct from a asset reference.
        Variant(const AssetRef& value) noexcept  // NOLINT(google-explicit-constructor)
        {
            *this = value;
        }

        /// Construct from a asset reference list.
        Variant(const AssetRefList& value) noexcept  // NOLINT(google-explicit-constructor)
        {
            *this = value;
        }

        /// Construct from a Vector2.
        Variant(const Vector2& value)
        {
            *this = value;
        }

        /// Construct from a Vector3.
        Variant(const Vector3& value)
        {
            *this = value;
        }

        /// Construct from a Vector4.
        Variant(const Vector4& value)
        {
            *this = value;
        }

        /// Construct from a Quaternion.
        Variant(const Quaternion& value) noexcept
        {
            *this = value;
        }

        /// Construct from a Color.
        Variant(const Color& value) noexcept
        {
            *this = value;
        }

#if TODO
        /// Construct from a Rect.
        Variant(const Rect& value) noexcept
        {
            *this = value;
        }

        /// Construct from a RectF.
        Variant(const RectF& value) noexcept
        {
            *this = value;
        }

        /// Construct from a Int2.
        Variant(const Int2& value) noexcept
        {
            *this = value;
        }

        /// Construct from a UInt2.
        Variant(const UInt2& value) noexcept
        {
            *this = value;
        }

        /// Construct from a Double2.
        Variant(const Double2& value) noexcept
        {
            *this = value;
        }
#endif // TODO

        /// Construct from type.
        Variant(VariantType type);

        /// Copy-construct from another variant.
        Variant(const Variant& value)
        {
            *this = value;
        }

        /// Move-construct from another variant.
        Variant(Variant&& value) noexcept
        {
            *this = std::move(value);
        }

        /// Destruct.
        ~Variant()
        {
            SetType(VariantType::None);
        }

        /// Reset to empty.
        void Clear()
        {
            SetType(VariantType::None);
        }

        /// Assign from another variant.
        Variant& operator =(const Variant& other);

        /// Move-assign from another variant.
        Variant& operator =(Variant&& rhs);

        /// Assign from an integer.
        Variant& operator =(bool rhs)
        {
            SetType(VariantType::Bool);
            value.boolValue = rhs;
            return *this;
        }

        /// Assign from an int8.
        Variant& operator =(int8_t rhs)
        {
            SetType(VariantType::Int8);
            value.int8 = rhs;
            return *this;
        }

        /// Assign from an uint8.
        Variant& operator =(uint8_t rhs)
        {
            SetType(VariantType::UInt8);
            value.uint8 = rhs;
            return *this;
        }

        /// Assign from an int16.
        Variant& operator =(int16_t rhs)
        {
            SetType(VariantType::Int16);
            value.int16 = rhs;
            return *this;
        }

        /// Assign from an uint16.
        Variant& operator =(uint16_t rhs)
        {
            SetType(VariantType::UInt16);
            value.uint16 = rhs;
            return *this;
        }

        /// Assign from an int32.
        Variant& operator =(int32_t rhs)
        {
            SetType(VariantType::Int32);
            value.int32 = rhs;
            return *this;
        }

        /// Assign from an uint32.
        Variant& operator =(uint32_t rhs)
        {
            SetType(VariantType::UInt32);
            value.uint32 = rhs;
            return *this;
        }

        /// Assign from 64 bit integer.
        Variant& operator =(int64_t rhs)
        {
            SetType(VariantType::Int64);
            value.int64 = rhs;
            return *this;
        }

        /// Assign from an unsigned 64 bit integer.
        Variant& operator =(uint64_t rhs)
        {
            SetType(VariantType::UInt64);
            value.uint64 = rhs;
            return *this;
        }

        /// Assign from a float.
        Variant& operator =(float rhs)
        {
            SetType(VariantType::Float);
            value.floatValue = rhs;
            return *this;
        }

        /// Assign from a double.
        Variant& operator =(double rhs)
        {
            SetType(VariantType::Double);
            value.doubleVal = rhs;
            return *this;
        }

        /// Assign from an enum value.
        template<typename TEnum, typename = typename std::enable_if_t<std::is_enum_v<TEnum>>>
        Variant& operator =(const TEnum enumValue)
        {
            SetType(VariantType::Enum);
            value.uint64 = ecast(enumValue);
            return *this;
        }

        /// Assign from a string.
        Variant& operator =(const String& other)
        {
            SetType(VariantType::String);
            value.string = other;
            return *this;
        }

        /// Assign from a C string.
        Variant& operator =(const char* other)
        {
            SetType(VariantType::String);
            value.string = other;
            return *this;
        }

        /// Assign from a StringHash (convert to integer).
        Variant& operator =(const StringId32& rhs)
        {
            SetType(VariantType::UInt32);
            value.uint32 = rhs.GetHash();
            return *this;
        }

        /// Assign from a byte vector.
        Variant& operator =(const ByteVector& rhs)
        {
            SetType(VariantType::ByteVector);
            value.byteVector = rhs;
            return *this;
        }

        /// Assign from a string vector.
        Variant& operator =(const StringVector& rhs)
        {
            SetType(VariantType::StringVector);
            value.stringVector = rhs;
            return *this;
        }

        /// Assign from a void pointer.
        Variant& operator =(void* rhs)
        {
            SetType(VariantType::Pointer);
            value.pointer = rhs;
            return *this;
        }

        /// Assign from a RefCounted pointer. The object will be stored internally in a WeakPtr so that its expiration can be detected safely.
        Variant& operator =(RefCounted* rhs)
        {
            SetType(VariantType::RefPtr);
            value.weakPtr = rhs;
            return *this;
        }

        /// Assign from a asset reference.
        Variant& operator =(const AssetRef& rhs)
        {
            SetType(VariantType::AssetRef);
            value.assetRef = rhs;
            return *this;
        }

        /// Assign from a asset reference list.
        Variant& operator =(const AssetRefList& rhs)
        {
            SetType(VariantType::AssetRefList);
            value.assetRefList = rhs;
            return *this;
        }


        /// Assign from a Vector2.
        Variant& operator =(const Vector2& rhs)
        {
            SetType(VariantType::Vector2);
            value.vector2 = rhs;
            return *this;
        }

        /// Assign from a Vector3.
        Variant& operator =(const Vector3& rhs)
        {
            SetType(VariantType::Vector3);
            value.vector3 = rhs;
            return *this;
        }

        /// Assign from a Vector4.
        Variant& operator =(const Vector4& rhs)
        {
            SetType(VariantType::Vector4);
            value.vector4 = rhs;
            return *this;
        }

        /// Assign from a Quaternion.
        Variant& operator =(const Quaternion& rhs)
        {
            SetType(VariantType::Quaternion);
            value.quaternion = rhs;
            return *this;
        }

        /// Assign from a Color.
        Variant& operator =(const Color& rhs)
        {
            SetType(VariantType::Color);
            value.color = rhs;
            return *this;
        }

#if TODO
        /// Assign from a Rect.
        Variant& operator =(const Rect& rhs)
        {
            SetType(VariantType::Rect);
            value.rect = rhs;
            return *this;
        }

        /// Assign from a RectF.
        Variant& operator =(const RectF& rhs)
        {
            SetType(VariantType::RectF);
            value.rectF = rhs;
            return *this;
        }

        /// Assign from a Int2.
        Variant& operator =(const Int2& rhs)
        {
            SetType(VariantType::Int2);
            int2 = rhs;
            return *this;
        }

        /// Assign from a UInt2.
        Variant& operator =(const UInt2& rhs)
        {
            SetType(VariantType::UInt2);
            uint2 = rhs;
            return *this;
        }

        /// Assign from a Double2.
        Variant& operator =(const Double2& rhs)
        {
            SetType(VariantType::Double2);
            double2 = rhs;
            return *this;
        }
#endif // TODO


        /// Test for equality with another variant.
        bool operator ==(const Variant& other) const;

        /// Test for equality with a bool. To return true, both the type and value must match.
        bool operator ==(bool rhs) const
        {
            return type == VariantType::Bool ? value.boolValue == rhs : false;
        }

        /// Test for equality with an integer. To return true, both the type and value must match.
        bool operator ==(int8_t rhs) const
        {
            return type == VariantType::Int8 ? value.int8 == rhs : false;
        }

        /// Test for equality with an unsigned 8 bit integer. To return true, both the type and value must match.
        bool operator ==(uint8_t rhs) const
        {
            return type == VariantType::UInt8 ? value.uint8 == rhs : false;
        }

        /// Test for equality with an integer. To return true, both the type and value must match.
        bool operator ==(int16_t rhs) const
        {
            return type == VariantType::Int16 ? value.int16 == rhs : false;
        }

        /// Test for equality with an unsigned 16 bit integer. To return true, both the type and value must match.
        bool operator ==(uint16_t rhs) const
        {
            return type == VariantType::UInt16 ? value.uint16 == rhs : false;
        }

        /// Test for equality with an integer. To return true, both the type and value must match.
        bool operator ==(int32_t rhs) const
        {
            return type == VariantType::Int32 ? value.int32 == rhs : false;
        }

        /// Test for equality with an unsigned 32 bit integer. To return true, both the type and value must match.
        bool operator ==(uint32_t rhs) const
        {
            return type == VariantType::UInt32 ? value.uint32 == rhs : false;
        }

        /// Test for equality with an int64_t. To return true, both the type and value must match.
        bool operator ==(int64_t rhs) const
        {
            return type == VariantType::Int64 ? value.int64 == rhs : false;
        }

        /// Test for equality with an uint64_t. To return true, both the type and value must match.
        bool operator ==(uint64_t rhs) const
        {
            return type == VariantType::UInt64 ? value.uint64 == rhs : false;
        }

        /// Test for equality with a float. To return true, both the type and value must match.
        bool operator ==(float rhs) const
        {
            return type == VariantType::Float ? value.floatValue == rhs : false;
        }

        /// Test for equality with a double. To return true, both the type and value must match.
        bool operator ==(double rhs) const
        {
            return type == VariantType::Double ? value.doubleVal == rhs : false;
        }

        /// Test for equality with a string. To return true, both the type and value must match.
        bool operator ==(const String& other) const
        {
            return type == VariantType::String ? value.string == other : false;
        }

        /// Test for equality with a c string. To return true, both the type and value must match.
        bool operator ==(const char* rhs) const
        {
            return type == VariantType::String ? value.string == rhs : false;
        }

        /// Test for equality with a byte vector. To return true, both the type and value must match.
        bool operator ==(const ByteVector& rhs) const
        {
            return type == VariantType::ByteVector ? value.byteVector == rhs : false;
        }

        /// Test for equality with a string vector. To return true, both the type and value must match.
        bool operator ==(const StringVector& rhs) const
        {
            return type == VariantType::StringVector ? value.stringVector == rhs : false;
        }

        /// Test for equality with a StringHash. To return true, both the type and value must match.
        bool operator ==(const StringId32& rhs) const
        {
            if (type == VariantType::Int32)
                return static_cast<uint32_t>(value.int32) == rhs.GetHash();
            else if (type == VariantType::UInt32)
                return static_cast<uint32_t>(value.uint32) == rhs.GetHash();

            return false;
        }

        /// Test for equality with a void pointer. To return true, both the type and value must match, with the exception that a RefCounted pointer is also allowed.
        bool operator ==(void* rhs) const
        {
            if (type == VariantType::Pointer)
                return value.pointer == rhs;
            else if (type == VariantType::RefPtr)
                return value.weakPtr == rhs;
            else
                return false;
        }

        /// Test for equality with a RefCounted pointer. To return true, both the type and value must match, with the exception that void pointer is also allowed.
        bool operator ==(RefCounted* rhs) const
        {
            if (type == VariantType::RefPtr)
                return value.weakPtr == rhs;
            else if (type == VariantType::Pointer)
                return value.pointer == rhs;
            else
                return false;
        }

        /// Test for equality with a asset reference. To return true, both the type and value must match.
        bool operator ==(const AssetRef& rhs) const
        {
            return type == VariantType::AssetRef ? value.assetRef == rhs : false;
        }

        /// Test for equality with a asset reference list. To return true, both the type and value must match.
        bool operator ==(const AssetRefList& rhs) const
        {
            return type == VariantType::AssetRefList ? value.assetRefList == rhs : false;
        }

        /// Test for equality with a Vector2. To return true, both the type and value must match.
        bool operator ==(const Vector2& rhs) const
        {
            return type == VariantType::Vector2 ? value.vector2 == rhs : false;
        }

        /// Test for equality with a Vector3. To return true, both the type and value must match.
        bool operator ==(const Vector3& rhs) const
        {
            return type == VariantType::Vector3 ? value.vector3 == rhs : false;
        }

        /// Test for equality with a Vector4. To return true, both the type and value must match.
        bool operator ==(const Vector4& rhs) const
        {
            return type == VariantType::Vector4 ? value.vector4 == rhs : false;
        }

        /// Test for equality with a Quaternion. To return true, both the type and value must match.
        bool operator ==(const Quaternion& rhs) const
        {
            return type == VariantType::Quaternion ? value.quaternion == rhs : false;
        }

        /// Test for equality with a Color. To return true, both the type and value must match.
        bool operator ==(const Color& rhs) const
        {
            return type == VariantType::Color ? value.color == rhs : false;
        }

#if TODO
        /// Test for equality with a Rect. To return true, both the type and value must match.
        bool operator ==(const Rect& rhs) const
        {
            return type == VariantType::Rect ? value.rect == rhs : false;
        }

        /// Test for equality with a RectF. To return true, both the type and value must match.
        bool operator ==(const RectF& rhs) const
        {
            return type == VariantType::RectF ? value.rectF == rhs : false;
        }

        /// Test for equality with a Int2. To return true, both the type and value must match.
        bool operator ==(const Int2& rhs) const
        {
            return type == VariantType::Int2 ? int2 == rhs : false;
        }

        /// Test for equality with a UInt2. To return true, both the type and value must match.
        bool operator ==(const UInt2& rhs) const
        {
            return type == VariantType::UInt2 ? uint2 == rhs : false;
        }

        /// Test for equality with a Double2. To return true, both the type and value must match.
        bool operator ==(const Double2& rhs) const
        {
            return type == VariantType::Double2 ? double2 == rhs : false;
        }
#endif // TODO


        /// Test for inequality with another variant.
        bool operator !=(const Variant& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a bool.
        bool operator !=(bool rhs) const { return !(*this == rhs); }

        /// Test for inequality with an integer.
        bool operator !=(int8_t rhs) const { return !(*this == rhs); }

        /// Test for inequality with an unsigned integer.
        bool operator !=(uint8_t rhs) const { return !(*this == rhs); }

        /// Test for inequality with an integer.
        bool operator !=(int16_t rhs) const { return !(*this == rhs); }

        /// Test for inequality with an unsigned integer.
        bool operator !=(uint16_t rhs) const { return !(*this == rhs); }

        /// Test for inequality with an integer.
        bool operator !=(int32_t rhs) const { return !(*this == rhs); }

        /// Test for inequality with an unsigned integer.
        bool operator !=(uint32_t rhs) const { return !(*this == rhs); }

        /// Test for inequality with an 64 bit integer.
        bool operator !=(int64_t rhs) const { return !(*this == rhs); }

        /// Test for inequality with an unsigned 64 bit integer.
        bool operator !=(uint64_t rhs) const { return !(*this == rhs); }

        /// Test for inequality with a float.
        bool operator !=(float rhs) const { return !(*this == rhs); }

        /// Test for inequality with a double.
        bool operator !=(double rhs) const { return !(*this == rhs); }

        /// Test for inequality with a string.
        bool operator !=(const String& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a const string.
        bool operator !=(const char* rhs) const { return !(*this == rhs); }

        /// Test for inequality with a byte vector.
        bool operator !=(const ByteVector& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a string vector.
        bool operator !=(const StringVector& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a StringId32.
        bool operator !=(const StringId32& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a pointer.
        bool operator !=(void* rhs) const { return !(*this == rhs); }

        /// Test for inequality with a RefCounted pointer.
        bool operator !=(RefCounted* rhs) const { return !(*this == rhs); }

        /// Test for inequality with a asset reference.
        bool operator !=(const AssetRef& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a asset reference list.
        bool operator !=(const AssetRefList& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a Vector2.
        bool operator !=(const Vector2& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a Vector3.
        bool operator !=(const Vector3& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a Vector4.
        bool operator !=(const Vector4& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a Quaternion.
        bool operator !=(const Quaternion& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a Color.
        bool operator !=(const Color& rhs) const { return !(*this == rhs); }

#if TODO
        /// Test for inequality with a Rect.
        bool operator !=(const Rect& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a RectF.
        bool operator !=(const RectF& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a Int2.
        bool operator !=(const Int2& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a UInt2.
        bool operator !=(const UInt2& rhs) const { return !(*this == rhs); }

        /// Test for inequality with a Double2.
        bool operator !=(const Double2& rhs) const { return !(*this == rhs); }
#endif // TODO


        /// Return value's type.
        VariantType GetType() const noexcept { return type; }

        /// Return value's type name.
        std::string GetTypeName() const;

        /// Return true when the variant value is considered zero according to its actual type.
        bool IsZero() const noexcept;

        /// Return true when the variant is empty (i.e. not initialized yet).
        bool IsEmpty() const noexcept { return type == VariantType::None; }

        /// Return true when the variant is enum value.
        bool IsEnum() const noexcept { return type == VariantType::Enum; }

        /// Return bool or false on type mismatch.
        bool GetBool() const noexcept;

        /// Return int8_t or zero on type mismatch. 
        int8_t GetInt8() const noexcept;

        /// Return uint8_t or zero on type mismatch. 
        uint8_t GetUInt8() const noexcept;

        /// Return int16_t or zero on type mismatch. 
        int16_t GetInt16() const noexcept;

        /// Return uint16_t or zero on type mismatch. 
        uint16_t GetUInt16() const noexcept;

        /// Return int or zero on type mismatch. Floats and doubles are converted.
        int32_t GetInt32() const noexcept;

        /// Return unsigned int or zero on type mismatch. Floats and doubles are converted.
        uint32_t GetUInt32() const noexcept;

        /// Return 64 bit int or zero on type mismatch. Floats and doubles are converted.
        int64_t GetInt64() const noexcept;

        /// Return unsigned 64 bit int or zero on type mismatch. Floats and doubles are converted.
        uint64_t GetUInt64() const noexcept;

        /// Return float or zero on type mismatch. Ints and doubles are converted.
        float GetFloat() const noexcept;

        /// Return double or zero on type mismatch. Ints and floats are converted.
        double GetDouble() const noexcept;

        /// Return string or empty on type mismatch.
        const String& GetString() const noexcept;

        /// Return a byte vector or empty on type mismatch.
        const ByteVector& GetByteVector() const noexcept;

        /// Return a string vector or empty on type mismatch.
        const StringVector& GetStringVector() const noexcept;

        /// Return StringId32 or zero on type mismatch.
        StringId32 GetStringId() const noexcept;

        /// Return void pointer or n ull on type mismatch. RefCounted pointer will be converted.
        void* GetVoidPtr() const noexcept;

        /// Return a RefCounted pointer or null on type mismatch. Will return null if holding a void pointer, as it can not be safely verified that the object is a RefCounted.
        RefCounted* GetPtr() const noexcept;

        /// Return a asset reference or empty on type mismatch.
        const AssetRef& GetAssetRef() const noexcept;

        /// Return a asset reference list or empty on type mismatch.
        const AssetRefList& GetAssetRefList() const noexcept;

        /// Return Vector2 or zero on type mismatch.
        const Vector2& GetVector2() const noexcept;

        /// Return Vector3 or zero on type mismatch.
        const Vector3& GetVector3() const noexcept;

        /// Return Vector4 or zero on type mismatch.
        const Vector4& GetVector4() const noexcept;

        /// Return quaternion or identity on type mismatch.
        const Quaternion& GetQuaternion() const noexcept;

        /// Return color or black on type mismatch. Vector4 is aliased to Color if necessary.
        const Color& GetColor() const noexcept;

#if TODO
        /// Return Rect or empty on type mismatch.
        const Rect& GetRect() const noexcept;

        /// Return RectF or empty on type mismatch.
        const RectF& GetRectF() const noexcept;

        /// Return Int2 or zero on type mismatch.
        const Int2& AsInt2() const
        {
            return type == VariantType::Int2 ? int2 : Int2::Zero;
        }

        /// Return UInt2 or zero on type mismatch.
        const UInt2& AsUInt2() const
        {
            return type == VariantType::UInt2 ? uint2 : UInt2::Zero;
        }

        /// Return Double2 or zero on type mismatch.
        const Double2& AsDouble2() const
        {
            return type == VariantType::Double2 ? double2 : Double2::Zero;
        }
#endif // TODO

        /// Return the value, template version.
        template <class T> T Get() const;

        /// Return the enum value, template version.
        template<typename TEnum, typename = typename std::enable_if_t<std::is_enum_v<TEnum>>>
        TEnum GetEnum() const
        {
            return static_cast<TEnum>(GetUInt64());
        }

        /// Empty variant.
        static const Variant Empty;

        /// Empty byte vector.
        static const ByteVector EmptyByteVector;

        /// Empty string vector.
        static const StringVector EmptyStringVector;

    private:
        /// Set new type and allocate/deallocate memory as necessary.
        void SetType(VariantType newType);

        /// Variant type.
        VariantType type = VariantType::None;
        /// Variant value.
        VariantValue value;
    };

    template <> ALIMER_API int8_t Variant::Get<int8_t>() const;
    template <> ALIMER_API uint8_t Variant::Get<uint8_t>() const;
    template <> ALIMER_API int16_t Variant::Get<int16_t>() const;
    template <> ALIMER_API uint16_t Variant::Get<uint16_t>() const;
    template <> ALIMER_API int32_t Variant::Get<int32_t>() const;
    template <> ALIMER_API uint32_t Variant::Get<uint32_t>() const;
    template <> ALIMER_API int64_t Variant::Get<int64_t>() const;
    template <> ALIMER_API uint64_t Variant::Get<uint64_t>() const;
    template <> ALIMER_API float Variant::Get<float>() const;
    template <> ALIMER_API double Variant::Get<double>() const;

    template <> ALIMER_API const String& Variant::Get<const String&>() const;
    template <> ALIMER_API String Variant::Get<String>() const;
    template <> ALIMER_API ByteVector Variant::Get<ByteVector>() const;
    template <> ALIMER_API StringVector Variant::Get<StringVector>() const;
    template <> ALIMER_API StringId32 Variant::Get<StringId32>() const;
    template <> ALIMER_API void* Variant::Get<void*>() const;
    template <> ALIMER_API RefCounted* Variant::Get<RefCounted*>() const;
    template <> ALIMER_API AssetRef Variant::Get<AssetRef>() const;
    template <> ALIMER_API AssetRefList Variant::Get<AssetRefList>() const;

    template <> ALIMER_API const Vector2& Variant::Get<const Vector2&>() const;
    template <> ALIMER_API Vector2 Variant::Get<Vector2>() const;
    template <> ALIMER_API const Vector3& Variant::Get<const Vector3&>() const;
    template <> ALIMER_API Vector3 Variant::Get<Vector3>() const;
    template <> ALIMER_API const Vector4& Variant::Get<const Vector4&>() const;
    template <> ALIMER_API Vector4 Variant::Get<Vector4>() const;
    template <> ALIMER_API const Quaternion& Variant::Get<const Quaternion&>() const;
    template <> ALIMER_API Quaternion Variant::Get<Quaternion>() const;
    template <> ALIMER_API const Color& Variant::Get<const Color&>() const;
    template <> ALIMER_API Color Variant::Get<Color>() const;

#if TODO
    template <> ALIMER_API const Rect& Variant::Get<const Rect&>() const;
    template <> ALIMER_API Rect Variant::Get<Rect>() const;
    template <> ALIMER_API const RectF& Variant::Get<const RectF&>() const;
    template <> ALIMER_API RectF Variant::Get<RectF>() const;

    template <> ALIMER_API const Int2& Variant::Get<const Int2&>() const;
    template <> ALIMER_API Int2 Variant::Get<Int2>() const;

    template <> ALIMER_API const UInt2& Variant::Get<const UInt2&>() const;
    template <> ALIMER_API UInt2 Variant::Get<UInt2>() const;

    template <> ALIMER_API const Double2& Variant::Get<const Double2&>() const;
    template <> ALIMER_API Double2 Variant::Get<Double2>() const;
#endif // TODO

}
