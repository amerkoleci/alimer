// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/Core/Object.h"
#include "Alimer/Core/SerializeValue.h"

using namespace Alimer;

PropertyInfo::PropertyInfo(const char* name, PropertyAccessor* accessor)
    : _name(name)
    , _accessor(accessor)
{

}

void PropertyInfo::Skip(VariantType type, Stream& source)
{
    const uint32_t size = GetVariantTypeSize(type);
    if (size)
    {
        source.Seek(source.GetPosition() + size);
        return;
    }

    switch (type)
    {
        case VariantType::String:
            source.ReadString();
            break;

        case VariantType::ObjectRef:
            source.ReadObjectRef();
            break;

        case VariantType::AssetRef:
            source.ReadAssetRef();
            break;

        case VariantType::AssetRefList:
            source.ReadAssetRefList();
            break;

        case VariantType::SerializeValue:
            source.ReadSerializeValue();
            break;

        default:
            break;
    }
}

void PropertyInfo::ToSerializeValue(VariantType type, SerializeValue& dest, const void* source)
{
    switch (type)
    {
        case VariantType::Bool:
            dest = *(reinterpret_cast<const bool*>(source));
            break;

        case VariantType::Int8:
            dest = *(reinterpret_cast<const int8_t*>(source));
            break;

        case VariantType::UInt8:
            dest = *(reinterpret_cast<const uint8_t*>(source));
            break;

        case VariantType::Int16:
            dest = *(reinterpret_cast<const int16_t*>(source));
            break;

        case VariantType::UInt16:
            dest = *(reinterpret_cast<const uint16_t*>(source));
            break;

        case VariantType::Int32:
            dest = *(reinterpret_cast<const int32_t*>(source));
            break;

        case VariantType::UInt32:
            dest = *(reinterpret_cast<const uint32_t*>(source));
            break;

        case VariantType::Int64:
            dest = *(reinterpret_cast<const int64_t*>(source));
            break;

        case VariantType::UInt64:
            dest = *(reinterpret_cast<const uint64_t*>(source));
            break;

        case VariantType::Float:
            dest = *(reinterpret_cast<const float*>(source));
            break;

        case VariantType::Double:
            dest = *(reinterpret_cast<const double*>(source));
            break;

        case VariantType::Enum:
            dest = *(reinterpret_cast<const uint64_t*>(source));
            break;

        case VariantType::String:
            dest = *(reinterpret_cast<const String*>(source));
            break;

        case VariantType::AssetRef:
            dest = reinterpret_cast<const AssetRef*>(source)->ToString();
            break;

        case VariantType::AssetRefList:
            dest = reinterpret_cast<const AssetRefList*>(source)->ToString();
            break;

        case VariantType::Vector2:
            dest.PushFixedFloatArray(reinterpret_cast<const Vector2*>(source)->data, 2);
            break;

        case VariantType::Vector3:
            dest.PushFixedFloatArray(reinterpret_cast<const Vector3*>(source)->data, 3);
            break;

        case VariantType::Vector4:
            dest.PushFixedFloatArray(reinterpret_cast<const Vector4*>(source)->data, 4);
            break;

        case VariantType::Quaternion:
            dest.PushFixedFloatArray(reinterpret_cast<const Quaternion*>(source)->data, 4);
            break;

        case VariantType::Color:
            dest.PushFixedFloatArray(&reinterpret_cast<const Color*>(source)->r, 4);
            break;

        default:
            ALIMER_ASSERT(false);
            break;
    }
}

void PropertyInfo::FromSerializeValue(VariantType type, void* dest, const SerializeValue& source)
{
    switch (type)
    {
        case VariantType::Bool:
            *(reinterpret_cast<bool*>(dest)) = source.GetBool();
            break;

        case VariantType::Int8:
            *(reinterpret_cast<int8_t*>(dest)) = static_cast<int8_t>(source.GetInt8());
            break;

        case VariantType::UInt8:
            *(reinterpret_cast<uint8_t*>(dest)) = static_cast<uint8_t>(source.GetUInt8());
            break;

        case VariantType::Int16:
            *(reinterpret_cast<int16_t*>(dest)) = static_cast<int16_t>(source.GetInt16());
            break;

        case VariantType::UInt16:
            *(reinterpret_cast<uint16_t*>(dest)) = static_cast<uint16_t>(source.GetUInt16());
            break;

        case VariantType::Int32:
            *(reinterpret_cast<int32_t*>(dest)) = static_cast<int32_t>(source.GetInt32());
            break;

        case VariantType::UInt32:
            *(reinterpret_cast<uint32_t*>(dest)) = static_cast<uint32_t>(source.GetUInt32());
            break;

        case VariantType::Int64:
            *(reinterpret_cast<int64_t*>(dest)) = static_cast<int64_t>(source.GetInt64());
            break;

        case VariantType::UInt64:
            *(reinterpret_cast<uint64_t*>(dest)) = static_cast<uint64_t>(source.GetUInt64());
            break;

        case VariantType::Float:
            *(reinterpret_cast<float*>(dest)) = static_cast<float>(source.GetFloat());
            break;

        case VariantType::Double:
            *(reinterpret_cast<double*>(dest)) = static_cast<double>(source.GetDouble());
            break;

        case VariantType::String:
            *(reinterpret_cast<String*>(dest)) = source.GetString();
            break;

        default:
            break;
    }
}

void PropertyInfo::GetValue(const Object* instance, void* dest) const
{
    _accessor->Get(instance, dest);
}

void PropertyInfo::SetValue(Object* instance, const void* value)
{
    _accessor->Set(instance, value);
}

