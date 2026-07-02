// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/Core/Object.h"
#include "Alimer/Core/SerializeValue.h"
//#include "Alimer/Serialization/Serializer.h"

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

#if TODO_SERIALIAZION
void PropertyInfo::Serialize(ISerializer& serializer, const std::string& propertyName, VariantType type, const void* source)
{
    switch (type)
    {
        case VariantType::Bool:
            serializer.Write(propertyName, *(reinterpret_cast<const bool*>(source)));
            break;

        case VariantType::Int8:
            serializer.Write(propertyName, *(reinterpret_cast<const int8_t*>(source)));
            break;

        case VariantType::UInt8:
            serializer.Write(propertyName, *(reinterpret_cast<const uint8_t*>(source)));
            break;

        case VariantType::Int16:
            serializer.Write(propertyName, *(reinterpret_cast<const int16_t*>(source)));
            break;

        case VariantType::UInt16:
            serializer.Write(propertyName, *(reinterpret_cast<const uint16_t*>(source)));
            break;

        case VariantType::Int32:
            serializer.Write(propertyName, *(reinterpret_cast<const int32_t*>(source)));
            break;

        case VariantType::UInt32:
            serializer.Write(propertyName, *(reinterpret_cast<const uint32_t*>(source)));
            break;

        case VariantType::Int64:
            serializer.Write(propertyName, *(reinterpret_cast<const int64_t*>(source)));
            break;

        case VariantType::UInt64:
            serializer.Write(propertyName, *(reinterpret_cast<const uint64_t*>(source)));
            break;

        case VariantType::Float:
            serializer.Write(propertyName, *(reinterpret_cast<const float*>(source)));
            break;

        case VariantType::Double:
            serializer.Write(propertyName, *(reinterpret_cast<const double*>(source)));
            break;

        case VariantType::Enum:
            serializer.Write(propertyName, *(reinterpret_cast<const uint64_t*>(source)));
            break;

        case VariantType::String:
            serializer.Write(propertyName, *(reinterpret_cast<const std::string*>(source)));
            break;

        case VariantType::AssetRef:
            serializer.Write(propertyName, (reinterpret_cast<const AssetRef*>(source)->ToString()));
            break;

        case VariantType::AssetRefList:
            serializer.Write(propertyName, (reinterpret_cast<const AssetRefList*>(source)->ToString()));
            break;

        case VariantType::Vector2:
            serializer.Write(propertyName, *(reinterpret_cast<const Vector2*>(source)));
            break;

        case VariantType::Vector3:
            serializer.Write(propertyName, *(reinterpret_cast<const Vector3*>(source)));
            break;

        case VariantType::Vector4:
            serializer.Write(propertyName, *(reinterpret_cast<const Vector4*>(source)));
            break;

        case VariantType::Quaternion:
            serializer.Write(propertyName, *(reinterpret_cast<const Quaternion*>(source)));
            break;

#if TODO
        case ATTR_INTVECTOR2:
            reinterpret_cast<IntVector2*>(dest)->FromString(source.GetString());
            break;

        case ATTR_INTVECTOR3:
            reinterpret_cast<IntVector3*>(dest)->FromString(source.GetString());
            break;

        case ATTR_INTRECT:
            reinterpret_cast<IntRect*>(dest)->FromString(source.GetString());
            break;

        case ATTR_INTBOX:
            reinterpret_cast<IntBox*>(dest)->FromString(source.GetString());
            break;
        case ATTR_QUATERNION:
            reinterpret_cast<Quaternion*>(dest)->FromString(source.GetString());
            break;

        case ATTR_COLOR:
            reinterpret_cast<Color*>(dest)->FromString(source.GetString());
            break;

        case ATTR_RECT:
            reinterpret_cast<Rect*>(dest)->FromString(source.GetString());
            break;

        case ATTR_BOUNDINGBOX:
            reinterpret_cast<BoundingBox*>(dest)->FromString(source.GetString());
            break;

        case ATTR_MATRIX3:
            reinterpret_cast<Matrix3*>(dest)->FromString(source.GetString());
            break;

        case ATTR_MATRIX3X4:
            reinterpret_cast<Matrix3x4*>(dest)->FromString(source.GetString());
            break;

        case ATTR_MATRIX4:
            reinterpret_cast<Matrix4*>(dest)->FromString(source.GetString());
            break;

        case ATTR_OBJECTREF:
            reinterpret_cast<ObjectRef*>(dest)->id = (unsigned)source.GetNumber();
            break;

        case ATTR_JSONVALUE:
            *(reinterpret_cast<SerializeValue*>(dest)) = source;
            break;
#endif // TODO


        default:
            ALIMER_ASSERT(false);
            break;
    }
}

bool PropertyInfo::Deserialize(IDeserializer& deserializer, const std::string& propertyName, VariantType type, void* dest)
{
    switch (type)
    {
        case VariantType::Bool:
            return deserializer.Read(propertyName, *(reinterpret_cast<bool*>(dest)));

        case VariantType::Int8:
            return deserializer.Read(propertyName, *(reinterpret_cast<int8_t*>(dest)));

        case VariantType::UInt8:
            return deserializer.Read(propertyName, *(reinterpret_cast<uint8_t*>(dest)));

        case VariantType::Int16:
            return deserializer.Read(propertyName, *(reinterpret_cast<int16_t*>(dest)));

        case VariantType::UInt16:
            return deserializer.Read(propertyName, *(reinterpret_cast<uint16_t*>(dest)));

        case VariantType::Int32:
            return deserializer.Read(propertyName, *(reinterpret_cast<int32_t*>(dest)));

        case VariantType::UInt32:
            return deserializer.Read(propertyName, *(reinterpret_cast<uint32_t*>(dest)));

        case VariantType::Int64:
            return deserializer.Read(propertyName, *(reinterpret_cast<int64_t*>(dest)));

        case VariantType::UInt64:
            return deserializer.Read(propertyName, *(reinterpret_cast<uint64_t*>(dest)));

        case VariantType::Float:
            return deserializer.Read(propertyName, *(reinterpret_cast<float*>(dest)));

        case VariantType::Double:
            return deserializer.Read(propertyName, *(reinterpret_cast<double*>(dest)));

        case VariantType::String:
            return deserializer.Read(propertyName, *(reinterpret_cast<std::string*>(dest)));
            break;

#if TODO_NEW
        case VariantType::Vector2:
            return deserializer.Read(propertyName, *(reinterpret_cast<Vector2*>(dest)));

        case VariantType::Vector3:
            return deserializer.Read(propertyName, *(reinterpret_cast<Vector3*>(dest)));

        case VariantType::Vector4:
            return deserializer.Read(propertyName, *(reinterpret_cast<Vector4*>(dest)));

        case VariantType::AssetRef:
            return deserializer.Read(propertyName, *(reinterpret_cast<AssetRef*>(dest)));

        case VariantType::AssetRefList:
            return deserializer.Read(propertyName, *(reinterpret_cast<AssetRefList*>(dest)));
#endif // TODO_NEW


#if TODO
        case ATTR_INTVECTOR2:
            reinterpret_cast<IntVector2*>(dest)->FromString(source.GetString());
            break;

        case ATTR_INTVECTOR3:
            reinterpret_cast<IntVector3*>(dest)->FromString(source.GetString());
            break;

        case ATTR_INTRECT:
            reinterpret_cast<IntRect*>(dest)->FromString(source.GetString());
            break;

        case ATTR_INTBOX:
            reinterpret_cast<IntBox*>(dest)->FromString(source.GetString());
            break;

        case ATTR_QUATERNION:
            reinterpret_cast<Quaternion*>(dest)->FromString(source.GetString());
            break;

        case ATTR_COLOR:
            reinterpret_cast<Color*>(dest)->FromString(source.GetString());
            break;

        case ATTR_RECT:
            reinterpret_cast<Rect*>(dest)->FromString(source.GetString());
            break;

        case ATTR_BOUNDINGBOX:
            reinterpret_cast<BoundingBox*>(dest)->FromString(source.GetString());
            break;

        case ATTR_MATRIX3:
            reinterpret_cast<Matrix3*>(dest)->FromString(source.GetString());
            break;

        case ATTR_MATRIX3X4:
            reinterpret_cast<Matrix3x4*>(dest)->FromString(source.GetString());
            break;

        case ATTR_MATRIX4:
            reinterpret_cast<Matrix4*>(dest)->FromString(source.GetString());
            break;

        case ATTR_STRING:
            *(reinterpret_cast<std::string*>(dest)) = source.GetString();
            break;

        case ATTR_OBJECTREF:
            reinterpret_cast<ObjectRef*>(dest)->id = (unsigned)source.GetNumber();
            break;

        case ATTR_JSONVALUE:
            *(reinterpret_cast<SerializeValue*>(dest)) = source;
            break;
#endif // TODO


        default:
            ALIMER_ASSERT(false);
            return false;
    }
}
#endif

void PropertyInfo::GetValue(const Object* instance, void* dest) const
{
    _accessor->Get(instance, dest);
}

void PropertyInfo::SetValue(Object* instance, const void* value)
{
    _accessor->Set(instance, value);
}

