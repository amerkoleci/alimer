// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Variant.h"
//#include "Alimer/Math/BoundingBox.h"
//#include "Alimer/Math/BoundingSphere.h"
//#include "Alimer/Math/Plane.h"
//#include "Alimer/Math/Matrix3x3.h"
//#include "Alimer/Math/Matrix4x3.h"

using namespace Alimer;

namespace
{
    static const char* typeNames[] =
    {
        "None",

        "Bool",
        "Int8",
        "UInt8",
        "Int16",
        "UInt16",
        "Int32",
        "UInt32",
        "Int64",
        "UInt64",
        "Float",
        "Double",
        "Enum",
        "String",
        "ByteVector",
        "StringVector",

        "Pointer",
        "RefPtr",
        "AssetRef",
        "AssetRefList",

        "Vector2",
        "Vector3",
        "Vector4",
        "Quaternion",
        "Color",
#if TODO
        "Rect",
        "RectF",
        "Int2",
        "Int3",
        "Int4",
        "UInt2",
        "UInt3",
        "UInt4",
        "Double2",
        "Double3",
        "Double4",

        "Rect",
        "RectF",
        "BoundingBox",
        "BoundingSphere",
        "Plane",
        "Ray",
        "Matrix3x2",
        "Matrix3x3",
        "Matrix3x4",
        "Matrix4x3",
        "Matrix4x4",
#endif // TODO

        nullptr
    };

    static const uint32_t typeSizeInBytes[] =
    {
        0,
        sizeof(bool),
        sizeof(int8_t),
        sizeof(uint8_t),
        sizeof(int16_t),
        sizeof(uint16_t),
        sizeof(int32_t),
        sizeof(uint32_t),
        sizeof(int64_t),
        sizeof(uint64_t),
        sizeof(float),
        sizeof(double),
        sizeof(uint64_t), // We store enum as uint_64_t
        sizeof(String),
        sizeof(ByteVector),
        sizeof(StringVector),

        0, // Pointer
        0, // RefPtr
        sizeof(AssetRef),
        sizeof(AssetRefList),

        sizeof(Vector2),
        sizeof(Vector3),
        sizeof(Vector4),
        sizeof(Quaternion),
        sizeof(Color),
#if TODO
        sizeof(Rect),
        sizeof(RectF),
        sizeof(Int2),
        sizeof(Int3),
        sizeof(Int4),
        sizeof(UInt2),
        sizeof(UInt3),
        sizeof(UInt4),
        sizeof(Double2),
        sizeof(Double3),
        sizeof(Double4),

        sizeof(Rect),
        sizeof(RectF),
        sizeof(BoundingBox),
        sizeof(BoundingSphere),
        sizeof(Plane),
        sizeof(Ray),
        sizeof(Matrix3x2),
        sizeof(Matrix3x3),
        sizeof(Matrix3x4),
        sizeof(Matrix4x3),
        sizeof(Matrix4x4),
#endif
        0
    };

    static_assert(sizeof(typeNames) / sizeof(const char*) == (size_t)VariantType::Count + 1, "Variant type name array is out-of-date");
    static_assert(sizeof(typeSizeInBytes) / sizeof(uint32_t) == (size_t)VariantType::Count + 1, "Variant type size array is out-of-date");
}

uint32_t Alimer::GetVariantTypeSize(VariantType type)
{
    return typeSizeInBytes[ecast(type)];
}

std::string Alimer::GetVariantTypeName(VariantType type)
{
    return typeNames[ecast(type)];
}

VariantType Alimer::GetVariantTypeFromName(std::string_view typeName)
{
    return (VariantType)StringUtils::GetStringListIndex(typeName.data(), typeNames, (uint32_t)VariantType::None);
}

const Variant Variant::Empty{ };
const ByteVector Variant::EmptyByteVector{ };
const StringVector Variant::EmptyStringVector{ };

Variant::Variant(VariantType type)
{
    SetType(type);
    switch (type)
    {
        case VariantType::Bool:
            *this = false;
            break;

        case VariantType::Int8:
            *this = (int8_t)0;
            break;

        case VariantType::UInt8:
            *this = (uint8_t)0;
            break;

        case VariantType::Int16:
            *this = (int16_t)0;
            break;

        case VariantType::UInt16:
            *this = (uint16_t)0;
            break;

        case VariantType::Int32:
            *this = (int32_t)0;
            break;

        case VariantType::UInt32:
            *this = (uint32_t)0u;
            break;

        case VariantType::Int64:
            *this = (int64_t)0;
            break;

        case VariantType::UInt64:
        case VariantType::Enum:
            *this = (uint64_t)0u;
            break;

        case VariantType::Float:
            *this = 0.f;
            break;

        case VariantType::Double:
            *this = 0.;
            break;

        case VariantType::String:
            *this = kEmptyString;
            break;

        case VariantType::Pointer:
            *this = (void*)nullptr;
            break;

        case VariantType::RefPtr:
            *this = (RefCounted*)nullptr;
            break;

        case VariantType::Vector2:
            *this = Vector2::Zero;
            break;

        case VariantType::Vector3:
            *this = Vector3::Zero;
            break;

        case VariantType::Vector4:
            *this = Vector4::Zero;
            break;

        case VariantType::Quaternion:
            *this = Quaternion::Identity;
            break;

        case VariantType::Color:
            *this = Colors::Black;
            break;
#if TODO
        case VariantType::Rect:
            *this = Rect::Empty;
            break;

        case VariantType::RectF:
            *this = RectF::Empty;
            break;

        case VAR_INTVECTOR2:
            *this = IntVector2::ZERO;
            break;

        case VAR_INTVECTOR3:
            *this = IntVector3::ZERO;
            break;

        case VAR_MATRIX3:
            *this = Matrix3::ZERO;
            break;

        case VAR_MATRIX3X4:
            *this = Matrix3x4::ZERO;
            break;

        case VAR_MATRIX4:
            *this = Matrix4::ZERO;
            break;

        case VAR_DOUBLE:
            *this = 0.0;
            break;

        case VAR_VARIANTCURVE:
            *this = emptyCurve;
            break;

        case VAR_VARIANTVECTOR:
        case VAR_VARIANTMAP:
        case VAR_STRINGVARIANTMAP:
            SetType(type);
            break;
#endif // TODO

        case VariantType::ByteVector:
        case VariantType::StringVector:
        case VariantType::AssetRef:
        case VariantType::AssetRefList:
            SetType(type);
            break;

        default:
            SetType(VariantType::None);
            break;
    }
}

void Variant::SetType(VariantType newType)
{
    if (type == newType)
        return;

    switch (type)
    {
        case VariantType::String:
            value.string.~basic_string<char>();
            break;

        case VariantType::ByteVector:
            value.byteVector.~ByteVector();
            break;

        case VariantType::StringVector:
            value.stringVector.~StringVector();
            break;

        case VariantType::RefPtr:
            value.weakPtr.~WeakPtr<RefCounted>();
            break;

        case VariantType::AssetRef:
            value.assetRef.~AssetRef();
            break;

        case VariantType::AssetRefList:
            value.assetRefList.~AssetRefList();
            break;

        default:
            break;
    }

    type = newType;

    switch (type)
    {
        case VariantType::String:
            new(&value.string) std::string();
            break;

        case VariantType::ByteVector:
            new(&value.byteVector) ByteVector();
            break;

        case VariantType::StringVector:
            new(&value.stringVector) StringVector();
            break;

        case VariantType::RefPtr:
            new(&value.weakPtr) WeakPtr<RefCounted>();
            break;

        case VariantType::AssetRef:
            new(&value.assetRef) AssetRef();
            break;

        case VariantType::AssetRefList:
            new(&value.assetRefList) AssetRefList();
            break;

        default:
            break;
    }
}

Variant& Variant::operator =(const Variant& other)
{
    // Assign other types here
    SetType(other.GetType());

    switch (type)
    {
        case VariantType::String:
            value.string = other.value.string;
            break;

        case VariantType::ByteVector:
            value.byteVector = other.value.byteVector;
            break;

        case VariantType::StringVector:
            value.stringVector = other.value.stringVector;
            break;

        case VariantType::RefPtr:
            value.weakPtr = other.value.weakPtr;
            break;

        case VariantType::AssetRef:
            value.assetRef = other.value.assetRef;
            break;

        case VariantType::AssetRefList:
            value.assetRefList = other.value.assetRefList;
            break;

        case VariantType::Vector2:
            value.vector2 = other.value.vector2;
            break;

        case VariantType::Vector3:
            value.vector3 = other.value.vector3;
            break;

        case VariantType::Vector4:
            value.vector4 = other.value.vector4;
            break;

        case VariantType::Quaternion:
            value.quaternion = other.value.quaternion;
            break;

        case VariantType::Color:
            value.color = other.value.color;
            break;

            //case VariantType::Rect:
            //    value.rect = other.value.rect;
            //    break;
            //
            //case VariantType::RectF:
            //    value.rectF = other.value.rectF;
            //    break;

        default:
            memcpy(value.storage, other.value.storage, sizeof(value.storage));
            break;
    }

    return *this;
}

Variant& Variant::operator =(Variant&& rhs)
{
    // Clear current value
    Clear();

    // Similar to SetType()
    // Call move-ctor for non-POD objects stored inplace.
    // Bitwise copy POD objects and objects stored by pointer.
    type = rhs.type;

    const auto moveConstruct = [&](auto& lhs, auto& rhs)
        {
            using Type = std::decay_t<decltype(lhs)>;
            new (&lhs) Type(std::move(rhs));
        };

    switch (type)
    {
        case VariantType::String:
            moveConstruct(value.string, rhs.value.string);
            break;

        case VariantType::ByteVector:
            moveConstruct(value.byteVector, rhs.value.byteVector);
            break;

        case VariantType::AssetRef:
            moveConstruct(value.assetRef, rhs.value.assetRef);
            break;

        case VariantType::AssetRefList:
            moveConstruct(value.assetRefList, rhs.value.assetRefList);
            break;

            //case VAR_VARIANTVECTOR: moveConstruct(value_.variantVector_, rhs.value_.variantVector_); break;

        case VariantType::StringVector:
            moveConstruct(value.stringVector, rhs.value.stringVector);
            break;

        case VariantType::RefPtr:
            moveConstruct(value.weakPtr, rhs.value.weakPtr);
            break;

        default:
            // Clear the moved object so it doesn't call destructor for its value.
            //memcpy(&value, &rhs.value, sizeof(VariantValue));
            memcpy(value.storage, rhs.value.storage, sizeof(value.storage));
            rhs.type = VariantType::None;
            break;
    }

    return *this;
}

std::string Variant::GetTypeName() const
{
    return typeNames[ecast(type)];
}

bool Variant::IsZero() const noexcept
{
    switch (type)
    {
        case VariantType::Bool:
            return value.boolValue == false;

        case VariantType::Int16:
            return value.int16 == 0;
        case VariantType::UInt16:
            return value.uint16 == 0;
        case VariantType::Int32:
            return value.int32 == 0;
        case VariantType::UInt32:
            return value.uint32 == 0;
        case VariantType::Int64:
            return value.int64 == 0;
        case VariantType::UInt64:
        case VariantType::Enum:
            return value.uint64 == 0;
        case VariantType::Float:
            return value.floatValue == 0.0f;
        case VariantType::Double:
            return value.doubleVal == 0.0;
        case VariantType::String:
            return value.string.empty();
        case VariantType::ByteVector:
            return value.byteVector.empty();
        case VariantType::StringVector:
            return value.stringVector.empty();

        case VariantType::Pointer:
            return value.pointer == nullptr;
        case VariantType::RefPtr:
            return value.weakPtr == nullptr;
        case VariantType::Vector2:
            return value.vector2.IsZero();
        case VariantType::Vector3:
            return value.vector3.IsZero();
        case VariantType::Vector4:
            return value.vector4.IsZero();
        case VariantType::Quaternion:
            return value.quaternion.IsIdentity();
        case VariantType::Color:
            return value.color == Colors::Black;
#if TODO
        case VariantType::Rect:
            return value.rect.IsEmpty();

        case VariantType::RectF:
            return value.rectF.IsEmpty();

        case VariantType::Int2:
            return _value.int2 == Int2::Zero;
        case VariantType::Int3:
            return _value.int3 == Int3::Zero;
        case VariantType::Int4:
            return _value.int4 == Int4::Zero;

        case VariantType::UInt2:
            return _value.uint2 == UInt2::Zero;
        case VariantType::UInt3:
            return _value.uint3 == UInt3::Zero;
        case VariantType::UInt4:
            return _value.uint4 == UInt4::Zero;

        case VariantType::Double2:
            return _value.double2 == Double2::Zero;
        case VariantType::Double3:
            return _value.double3 == Double3::Zero;
        case VariantType::Double4:
            return _value.double4 == Double4::Zero;
#endif // TODO


        default:
            return true;
    }
}

bool Variant::GetBool() const noexcept
{
    switch (type)
    {
        case VariantType::Bool:
            return value.boolValue;
        case VariantType::Int8:
            return value.int8 != 0;
        case VariantType::UInt8:
            return value.uint8 != 0;
        case VariantType::Int16:
            return value.int16 != 0;
        case VariantType::UInt16:
            return value.uint16 != 0;
        case VariantType::Int32:
            return value.int32 != 0;
        case VariantType::UInt32:
            return value.uint32 != 0;
        case VariantType::Int64:
            return value.int64 != 0;
        case VariantType::UInt64:
        case VariantType::Enum:
            return value.uint64 != 0;
        case VariantType::Float:
            return !MathF::IsZero(value.floatValue);
        case VariantType::Double:
            return !MathD::IsZero(value.doubleVal);
        case VariantType::String:
            return !value.string.empty();
        case VariantType::Pointer:
            return value.pointer;
        case VariantType::RefPtr:
            return value.weakPtr;
        default:
            return false;
    }
}

int8_t Variant::GetInt8() const noexcept
{
    return type == VariantType::Int8 ? value.int8 : false;
}

uint8_t Variant::GetUInt8() const noexcept
{
    return type == VariantType::UInt8 ? value.uint8 : false;
}

int16_t Variant::GetInt16() const noexcept
{
    return type == VariantType::Int16 ? value.int16 : false;
}

uint16_t Variant::GetUInt16() const noexcept
{
    return type == VariantType::UInt16 ? value.uint16 : false;
}

int32_t Variant::GetInt32() const noexcept
{
    if (type == VariantType::Int32)
        return value.int32;
    else if (type == VariantType::Int16)
        return value.int16;
    else if (type == VariantType::Int8)
        return value.int8;
    else if (type == VariantType::Float)
        return static_cast<int32_t>(value.floatValue);
    else if (type == VariantType::Double)
        return static_cast<int32_t>(value.doubleVal);
    else
        return 0;
}

uint32_t Variant::GetUInt32() const noexcept
{
    if (type == VariantType::UInt32)
        return value.uint32;
    else if (type == VariantType::UInt16)
        return value.uint16;
    else if (type == VariantType::UInt8)
        return value.uint8;
    else if (type == VariantType::Float)
        return static_cast<uint32_t>(value.floatValue);
    else if (type == VariantType::Double)
        return static_cast<uint32_t>(value.doubleVal);

    return 0;
}

int64_t Variant::GetInt64() const noexcept
{
    if (type == VariantType::Int64)
        return value.int64;
    else if (type == VariantType::Int32)
        return value.int32;
    else if (type == VariantType::Int16)
        return value.int16;
    else if (type == VariantType::Int8)
        return value.int8;
    else if (type == VariantType::Float)
        return static_cast<int64_t>(value.floatValue);
    else if (type == VariantType::Double)
        return static_cast<int64_t>(value.doubleVal);
    else
        return 0;
}

uint64_t Variant::GetUInt64() const noexcept
{

    if (type == VariantType::UInt64 || type == VariantType::Enum)
        return value.uint64;
    else if (type == VariantType::UInt32)
        return value.uint32;
    else if (type == VariantType::UInt16)
        return value.uint16;
    else if (type == VariantType::UInt8)
        return value.uint8;
    else if (type == VariantType::Float)
        return static_cast<uint64_t>(value.floatValue);
    else if (type == VariantType::Double)
        return static_cast<uint64_t>(value.doubleVal);

    return 0;
}

float Variant::GetFloat() const noexcept
{
    if (type == VariantType::Float)
        return value.floatValue;
    else if (type == VariantType::Double)
        return static_cast<float>(value.doubleVal);
    else if (type == VariantType::Int8)
        return static_cast<float>(value.int8);
    else if (type == VariantType::Int16)
        return static_cast<float>(value.int16);
    else if (type == VariantType::Int32)
        return static_cast<float>(value.int32);
    else if (type == VariantType::Int64)
        return static_cast<float>(value.int64);

    return 0.0f;
}

double Variant::GetDouble() const noexcept
{
    if (type == VariantType::Double)
        return value.doubleVal;
    else if (type == VariantType::Float)
        return value.floatValue;
    else if (type == VariantType::Int8)
        return static_cast<double>(value.int8);
    else if (type == VariantType::Int16)
        return static_cast<double>(value.int16);
    else if (type == VariantType::Int32)
        return static_cast<double>(value.int32);
    else if (type == VariantType::Int64)
        return static_cast<double>(value.int64);

    return 0.0;
}

const String& Variant::GetString() const noexcept
{
    return type == VariantType::String ? value.string : kEmptyString;
}

const ByteVector& Variant::GetByteVector() const noexcept
{
    return type == VariantType::ByteVector ? value.byteVector : EmptyByteVector;
}

const StringVector& Variant::GetStringVector() const noexcept
{
    return type == VariantType::StringVector ? value.stringVector : EmptyStringVector;
}

StringId32 Variant::GetStringId() const noexcept
{
    switch (type)
    {
        case VariantType::UInt8:
        case VariantType::UInt16:
        case VariantType::UInt32:
        case VariantType::Float:
        case VariantType::Double:
            return StringId32(GetUInt32());
        default:
            return StringId32::Empty;
    }
}

void* Variant::GetVoidPtr() const noexcept
{
    if (type == VariantType::Pointer)
        return value.pointer;
    else if (type == VariantType::RefPtr)
        return value.weakPtr;

    return nullptr;
}

RefCounted* Variant::GetPtr() const noexcept
{
    return type == VariantType::RefPtr ? value.weakPtr : nullptr;
}

const AssetRef& Variant::GetAssetRef() const noexcept
{
    return type == VariantType::AssetRef ? value.assetRef : AssetRef::Empty;
}

const AssetRefList& Variant::GetAssetRefList() const noexcept
{
    return type == VariantType::AssetRefList ? value.assetRefList : AssetRefList::Empty;
}

const Vector2& Variant::GetVector2() const noexcept
{
    return type == VariantType::Vector2 ? value.vector2 : Vector2::Zero;
}

const Vector3& Variant::GetVector3() const noexcept
{
    return type == VariantType::Vector3 ? value.vector3 : Vector3::Zero;
}

const Vector4& Variant::GetVector4() const noexcept
{
    return type == VariantType::Vector4 ? value.vector4 : Vector4::Zero;
}

const Quaternion& Variant::GetQuaternion() const noexcept
{
    return type == VariantType::Quaternion ? value.quaternion : Quaternion::Identity;
}

const Color& Variant::GetColor() const noexcept
{
    return (type == VariantType::Color || type == VariantType::Vector4) ? value.color : Colors::Black;
}

#if TODO
const Rect& Variant::GetRect() const noexcept
{
    return type == VariantType::Rect ? value.rect : Rect::Empty;
}

const RectF& Variant::GetRectF() const noexcept
{
    return type == VariantType::RectF ? value.rectF : RectF::Empty;
}
#endif // TODO

bool Variant::operator ==(const Variant& rhs) const
{
    if (type == VariantType::Pointer || type == VariantType::RefPtr)
        return GetVoidPtr() == rhs.GetVoidPtr();
    //else if (type == VAR_CUSTOM && rhs.type == VAR_CUSTOM)
    //    return GetCustomVariantValuePtr()->Compare(*rhs.GetCustomVariantValuePtr());
    else if (type != rhs.type)
        return false;

    switch (type)
    {
        case VariantType::Bool:
            return value.boolValue == rhs.value.boolValue;

        case VariantType::Int8:
            return value.int8 == rhs.value.int8;

        case VariantType::UInt8:
            return value.uint8 == rhs.value.uint8;

        case VariantType::Int16:
            return value.int16 == rhs.value.int16;

        case VariantType::UInt16:
            return value.uint16 == rhs.value.uint16;

        case VariantType::Int32:
            return value.int32 == rhs.value.int32;

        case VariantType::UInt32:
            return value.uint32 == rhs.value.uint32;

        case VariantType::Int64:
            return value.int64 == rhs.value.int64;

        case VariantType::UInt64:
        case VariantType::Enum:
            return value.uint64 == rhs.value.uint64;

        case VariantType::Float:
            return value.floatValue == rhs.value.floatValue;

        case VariantType::Double:
            return value.doubleVal == rhs.value.doubleVal;

        case VariantType::String:
            return value.string == rhs.value.string;

        case VariantType::ByteVector:
            return value.byteVector == rhs.value.byteVector;

        case VariantType::StringVector:
            return value.stringVector == rhs.value.stringVector;

        case VariantType::AssetRef:
            return value.assetRef == rhs.value.assetRef;

        case VariantType::AssetRefList:
            return value.assetRefList == rhs.value.assetRefList;

        case VariantType::Vector2:
            return value.vector2 == rhs.value.vector2;

        case VariantType::Vector3:
            return value.vector3 == rhs.value.vector3;

        case VariantType::Vector4:
            return value.vector4 == rhs.value.vector4;

        case VariantType::Quaternion:
            return value.quaternion == rhs.value.quaternion;

        case VariantType::Color:
            return value.color == rhs.value.color;
#if TODO

        case VariantType::Rect:
            return value.rect == rhs.value.rect;

        case VariantType::RectF:
            return value.rectF == rhs.value.rectF;

        case VariantType::Int2:
            return _value.int2 == rhs._value.int2;

        case VariantType::Int3:
            return _value.int3 == rhs._value.int3;

        case VariantType::Int4:
            return _value.int4 == rhs._value.int4;

        case VariantType::UInt2:
            return _value.uint2 == rhs._value.uint2;

        case VariantType::UInt3:
            return _value.uint3 == rhs._value.uint3;

        case VariantType::UInt4:
            return _value.uint4 == rhs._value.uint4;

        case VariantType::Double2:
            return _value.double2 == rhs._value.double2;

        case VariantType::Double3:
            return _value.double3 == rhs._value.double3;

        case VariantType::Double4:
            return _value.double4 == rhs._value.double4;
#endif // TODO


        default:
            return true;
    }
}

template <> int8_t Variant::Get<int8_t>() const
{
    return GetInt8();
}

template <> uint8_t Variant::Get<uint8_t>() const
{
    return GetUInt8();
}

template <> int16_t Variant::Get<int16_t>() const
{
    return GetInt16();
}

template <> uint16_t Variant::Get<uint16_t>() const
{
    return GetUInt16();
}

template <> int32_t Variant::Get<int32_t>() const
{
    return GetInt32();
}

template <> uint32_t Variant::Get<uint32_t>() const
{
    return GetUInt32();
}

template <> int64_t Variant::Get<int64_t>() const
{
    return GetInt64();
}

template <> uint64_t Variant::Get<uint64_t>() const
{
    return GetUInt64();
}

template <> float Variant::Get<float>() const
{
    return GetFloat();
}

template <> double Variant::Get<double>() const
{
    return GetDouble();
}

template <> const String& Variant::Get<const String&>() const
{
    return GetString();
}

template <> String Variant::Get<String>() const
{
    return GetString();
}

template <> ByteVector Variant::Get<ByteVector>() const
{
    return GetByteVector();
}

template <> StringVector Variant::Get<StringVector>() const
{
    return GetStringVector();
}

template <> StringId32 Variant::Get<StringId32>() const
{
    return GetStringId();
}

template <> void* Variant::Get<void*>() const
{
    return GetVoidPtr();
}

template <> RefCounted* Variant::Get<RefCounted*>() const
{
    return GetPtr();
}

template <> AssetRef Variant::Get<AssetRef>() const
{
    return GetAssetRef();
}

template <> AssetRefList Variant::Get<AssetRefList>() const
{
    return GetAssetRefList();
}

template <> const Vector2& Variant::Get<const Vector2&>() const
{
    return GetVector2();
}

template <> Vector2 Variant::Get<Vector2>() const
{
    return GetVector2();
}

template <> const Vector3& Variant::Get<const Vector3&>() const
{
    return GetVector3();
}

template <> Vector3 Variant::Get<Vector3>() const
{
    return GetVector3();
}

template <> const Vector4& Variant::Get<const Vector4&>() const
{
    return GetVector4();
}

template <> Vector4 Variant::Get<Vector4>() const
{
    return GetVector4();
}

template <> const Quaternion& Variant::Get<const Quaternion&>() const
{
    return GetQuaternion();
}

template <> Quaternion Variant::Get<Quaternion>() const
{
    return GetQuaternion();
}

template <> const Color& Variant::Get<const Color&>() const
{
    return GetColor();
}

template <> Color Variant::Get<Color>() const
{
    return GetColor();
}

#if TODO
template <> const Rect& Variant::Get<const Rect&>() const
{
    return GetRect();
}

template <> Rect Variant::Get<Rect>() const
{
    return GetRect();
}

template <> const Int2& Variant::Get<const Int2&>() const
{
    return AsInt2();
}

template <> Int2 Variant::Get<Int2>() const
{
    return AsInt2();
}

template <> const UInt2& Variant::Get<const UInt2&>() const
{
    return AsUInt2();
}

template <> UInt2 Variant::Get<UInt2>() const
{
    return AsUInt2();
}

template <> const Double2& Variant::Get<const Double2&>() const
{
    return AsDouble2();
}

template <> Double2 Variant::Get<Double2>() const
{
    return AsDouble2();
}
#endif // TODO

