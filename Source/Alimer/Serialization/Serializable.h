// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Vector.h"
#include "Alimer/Core/Object.h"
#include "Alimer/Core/Property.h"

namespace Alimer
{
    class ISerializer;
    class IDeserializer;

    using PropertyVector = Vector<SharedPtr<PropertyInfo>>;

    /// Base class for objects with automatic serialization using properties.
    class ALIMER_API Serializable : public Object
    {
        ALIMER_OBJECT(Serializable, Object);

    public:
        /// Serialize.
        //virtual void Serialize(ISerializer& serializer);

        /// Deserialize.
        //virtual void Deserialize(IDeserializer& deserializer);

        /// Load from binary stream. Store object ref attributes to be resolved later.
        virtual void Load(Stream& source/*, ObjectResolver& resolver*/);

        /// Save to binary stream.
        virtual void Save(Stream& dest);

        /// Return properties descriptions, or null if none defined.
        virtual const PropertyVector* GetProperties() const;

        /// Return an property info by name, or null if does not exist.
        PropertyInfo* GetProperty(const std::string& name) const;

        /// Set property value from memory.
        void SetPropertyValue(PropertyInfo* property, const void* source);

        /// Set property value from memory.
        bool SetPropertyValue(const std::string& name, const void* source);

        /// Get property value to memory.
        void GetPropertyValue(PropertyInfo* property, void* dest);

        /// Get property value to memory.
        bool GetPropertyValue(const std::string& name, void* dest);

        /// Set property value, template version. Return true if value was right type.
        template <class T> bool SetPropertyValue(PropertyInfo* property, const T& source)
        {
            PropertyInfoImpl<T>* typedProperty = dynamic_cast<PropertyInfoImpl<T>*>(property);
            if (typedProperty != nullptr)
            {
                typedProperty->SetValue(this, source);
                return true;
            }

            return false;
        }

        /// Set property value, template version. Return true if properties .
        template <class T> bool SetPropertyValue(const std::string& name, const T& source)
        {
            auto property = GetProperty(name);
            if (property != nullptr)
            {
                return SetPropertyValue(property, source);
            }

            return false;
        }

        /// Copy property value, template version. Return true if value was right type.
        template <class T> bool GetPropertyValue(PropertyInfo* property, T& dest)
        {
            PropertyInfoImpl<T>* typedProperty = dynamic_cast<PropertyInfoImpl<T>*>(property);
            if (typedProperty != nullptr)
            {
                typedProperty->GetValue(this, dest);
                return true;
            }

            return false;
        }

        /// Return property value, template version.
        template <class T> T GetPropertyValue(PropertyInfo* property)
        {
            PropertyInfoImpl<T>* typedProperty = dynamic_cast<PropertyInfoImpl<T>*>(property);
            return typedProperty != nullptr ? typedProperty->GetValue(this) : T();
        }

        /// Copy property value, template version. Return true if value was right type.
        template <class T> bool GetPropertyValue(const std::string& name, T& dest)
        {
            auto property = GetProperty(name);
            if (property != nullptr)
            {
                return GetPropertyValue(property, dest);
            }

            return false;
        }

        /// Return property value, template version.
        template <class T> T GetPropertyValue(const std::string& name, const T& defaultValue = T())
        {
            auto property = GetProperty(name);
            if (property != nullptr)
            {
                T result;
                GetPropertyValue(property, &result);
                return result;
            }

            return defaultValue;
        }

        /// Register a per-class property. If a property with the same name already exists, it will be replaced.
        static void RegisterProperty(const StringId32& type, PropertyInfo* property);
        /// Copy all base class properties.
        static void CopyBaseProperties(const StringId32& type, const StringId32& baseType);
        /// Copy one base class property.
        static void CopyBaseProperty(const StringId32& type, const StringId32& baseType, const std::string& name);

        /// Register a per-class property, template version. Should not be used for base class properties unless the type is explicitly specified, as by default the property will be re-registered to the base class redundantly.
        template <typename T, typename U>
        static void RegisterProperty(const char* name, U(T::* getFunction)() const, void (T::* setFunction)(U), const U& defaultValue = U(), const char** enumNames = 0)
        {
            RegisterProperty(T::GetTypeStatic(), new PropertyInfoImpl<U>(name, new PropertyAccessorImpl<T, U>(getFunction, setFunction), defaultValue, enumNames));
        }

        //template<typename T, typename TEnum, typename = typename std::enable_if<std::is_enum<TEnum>::value>::type>
        //static void RegisterEnumProperty(const char* name, TEnum(T::* getFunction)() const, void (T::* setFunction)(TEnum), const TEnum& defaultValue = TEnum(), const char** enumNames = 0)
        //{
        //    RegisterProperty(T::GetTypeStatic(), new PropertyInfoImpl<std::underlying_type<T>::type>(name, new PropertyAccessorImpl<T, TEnum>(getFunction, setFunction), defaultValue, enumNames));
        //}

        /// Register a per-class property with reference access, template version. Should not be used for base class properties unless the type is explicitly specified, as by default the property will be re-registered to the base class redundantly.
        template <typename T, typename U> static void RegisterRefProperty(const char* name, const U& (T::* getFunction)() const, void (T::* setFunction)(const U&), const U& defaultValue = U(), const char** enumNames = 0)
        {
            RegisterProperty(T::GetTypeStatic(), new PropertyInfoImpl<U>(name, new RefPropertyAccessorImpl<T, U>(getFunction, setFunction), defaultValue, enumNames));
        }

        /// Register a per-class attribute with mixed reference access, template version. Should not be used for base class attributes unless the type is explicitly specified, as by default the attribute will be re-registered to the base class redundantly.
        template <class T, class U> static void RegisterMixedRefProperty(const char* name, U(T::* getFunction)() const, void (T::* setFunction)(const U&), const U& defaultValue = U(), const char** enumNames = 0)
        {
            RegisterProperty(T::GetTypeStatic(), new PropertyInfoImpl<U>(name, new MixedRefPropertyAccessorImpl<T, U>(getFunction, setFunction), defaultValue, enumNames));
        }

        /// Copy all base class properties, template version.
        template <class Type, class TBaseType> static void CopyBaseProperties()
        {
            CopyBaseProperties(Type::GetTypeStatic(), TBaseType::GetTypeStatic());
        }

        /// Copy one base class property, template version.
        template <class Type, class TBaseType> static void CopyBaseProperty(const std::string& name)
        {
            CopyBaseProperty(Type::GetTypeStatic(), TBaseType::GetTypeStatic(), name);
        }
    };
}
