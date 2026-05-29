// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/TypeInfo.h"

namespace Alimer
{
    class Event;

    /// Base class for objects with type identification
    class ALIMER_API Object : public RefCounted
    {
    public:
        /// Constructor.
        Object() = default;
        /// Destructor.
        virtual ~Object() = default;

        /// Return type hash.
        virtual StringId32 GetType() const = 0;
        /// Return type name.
        virtual const std::string& GetTypeName() const = 0;
        /// Return type info.
        virtual const TypeInfo* GetTypeInfo() const = 0;

        /// Return type info static.
        static const TypeInfo* GetTypeInfoStatic() { return nullptr; }

        /// Check current instance is type of specified type.
        bool IsInstanceOf(StringId32 type) const;
        /// Check current instance is type of specified type.
        bool IsInstanceOf(const TypeInfo* typeInfo) const;
        /// Check current instance is type of specified class.
        template <typename T> bool IsInstanceOf() const { return IsInstanceOf(T::GetTypeInfoStatic()); }
        /// Cast the object to specified most derived class.
        template <typename T> T* Cast() { return IsInstanceOf<T>() ? static_cast<T*>(this) : nullptr; }
        /// Cast the object to specified most derived class.
        template <typename T> const T* Cast() const
        {
            return IsInstanceOf<T>() ? static_cast<const T*>(this) : nullptr;
        }

        /// Return properties descriptions, or null if none defined.
        const PropertyVector& GetProperties() const;

        /// Return an property info by name, or null if does not exist.
        PropertyInfo* GetProperty(StringView name) const;

        /// Get property value to memory.
        void GetPropertyValue(PropertyInfo* property, void* dest);

        /// Get property value to memory.
        bool GetPropertyValue(StringView name, void* dest);

        /// Set property value from memory.
        void SetPropertyValue(PropertyInfo* property, const void* source);

        /// Set property value from memory.
        bool SetPropertyValue(StringView name, const void* source);

        /// Copy property value, template version. Return true if value was right type.
        template <typename T>
        bool GetPropertyValue(PropertyInfo* property, T& dest)
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
        template <typename T>
        [[nodiscard]] T GetPropertyValue(PropertyInfo* property)
        {
            PropertyInfoImpl<T>* typedProperty = dynamic_cast<PropertyInfoImpl<T>*>(property);
            return typedProperty != nullptr ? typedProperty->GetValue(this) : T();
        }

        /// Copy property value, template version. Return true if value was right type.
        template <typename T>
        bool GetPropertyValue(StringView name, T& dest)
        {
            auto property = GetProperty(name);
            if (property != nullptr)
            {
                return GetPropertyValue(property, dest);
            }

            return false;
        }

        /// Return property value, template version.
        template <typename T>
        [[nodiscard]] T GetPropertyValue(StringView name, const T& defaultValue = T())
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

        /// Set property value, template version. Return true if value was right type.
        template <typename T>
        bool SetPropertyValue(PropertyInfo* property, const T& value)
        {
            PropertyInfoImpl<T>* typedProperty = dynamic_cast<PropertyInfoImpl<T>*>(property);
            if (typedProperty != nullptr)
            {
                typedProperty->SetValue(this, value);
                return true;
            }

            return false;
        }

        /// Set property value, template version. Return true if properties .
        template <typename T>
        bool SetPropertyValue(StringView name, const T& source)
        {
            auto property = GetProperty(name);
            if (property != nullptr)
            {
                return SetPropertyValue(property, source);
            }

            return false;
        }

        /// Set property value from StringView value.
        bool SetPropertyValue(StringView name, StringView value);

        /// Set property value from const char* value.
        bool SetPropertyValue(StringView name, const char* value);
    };
}

#define ALIMER_OBJECT(typeName, baseTypeName) \
public: \
    using Parent = baseTypeName; \
    virtual Alimer::StringId32 GetType() const override { return GetTypeInfoStatic()->GetType(); } \
    virtual const std::string& GetTypeName() const override { return GetTypeInfoStatic()->GetTypeName(); } \
    virtual const Alimer::TypeInfo* GetTypeInfo() const override { return GetTypeInfoStatic(); }  \
    static Alimer::StringId32 GetTypeStatic() { return GetTypeInfoStatic()->GetType(); } \
    static const std::string& GetTypeNameStatic() { return GetTypeInfoStatic()->GetTypeName(); } \
    static const Alimer::TypeInfo* GetTypeInfoStatic() \
    { \
        static const Alimer::TypeInfo typeInfoStatic(#typeName, Parent::GetTypeInfoStatic()); \
        return &typeInfoStatic; \
    }
