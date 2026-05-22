// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Property.h"

namespace Alimer
{
    /// Type info.
    class ALIMER_API TypeInfo final
    {
    public:
        /// Construct.
        TypeInfo(const char* typeName, const TypeInfo* baseTypeInfo);
        /// Destruct.
        ~TypeInfo() = default;

        /// Check current type is type of specified type.
        [[nodiscard]] bool IsTypeOf(StringId32 type) const;
        /// Check current type is type of specified type.
        [[nodiscard]] bool IsTypeOf(const TypeInfo* typeInfo) const;
        /// Check current type is type of specified class type.
        template <typename T> [[nodiscard]] bool IsTypeOf() const { return IsTypeOf(T::GetTypeInfoStatic()); }

        /// Return type.
        [[nodiscard]] StringId32 GetType() const { return type; }
        /// Return type name.
        [[nodiscard]] const std::string& GetTypeName() const { return typeName; }
        /// Return base type info.
        [[nodiscard]] const TypeInfo* GetBaseTypeInfo() const { return baseTypeInfo; }

    private:
        /// Type.
        StringId32 type;
        /// Type name.
        std::string typeName;
        /// Base class type info.
        const TypeInfo* baseTypeInfo;
    };

    class Event;
    class ObjectFactory;
    template <class T> class ObjectFactoryImpl;
    template <class T> class ObjectCreateFactoryImpl;

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

        /// Register an object factory.
        static void RegisterFactory(ObjectFactory* factory);
        /// Create an object by type hash. Return pointer to it or null if no factory found.
        static SharedPtr<Object> CreateObject(StringId32 type);

        /// Register an object factory, template version.
        template <class T> static inline void RegisterFactory()
        {
            RegisterFactory(new ObjectFactoryImpl<T>());
        }

        /// Create and return an object through a factory, template version.
        template <class T> static inline SharedPtr<T> CreateObject()
        {
            return StaticCast<T>(CreateObject(T::GetTypeStatic()));
        }

        /// Create and return an object through a factory, template version with given type.
        template <class T> static inline SharedPtr<T> CreateObject(StringId32 type)
        {
            return StaticCast<T>(CreateObject(type));
        }

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

        /// Return a type name from hash, or empty if not known. Requires a registered object factory.
        static std::string GetTypeNameFromType(StringId32 type);
    };

    /// Base class for object factories.
    class ALIMER_API ObjectFactory
    {
    public:
        /// Destructor.
        virtual ~ObjectFactory() = default;

        /// /// Create an object.
        virtual SharedPtr<Object> CreateObject() = 0;

        /// Return type info of objects created by this factory.
        const TypeInfo* GetTypeInfo() const { return _typeInfo; }

        /// Return type hash of objects created by this factory.
        StringId32 GetType() const { return _typeInfo->GetType(); }

        /// Return type name of objects created by this factory.
        const std::string& GetTypeName() const { return _typeInfo->GetTypeName(); }

    protected:
        /// Type info.
        const TypeInfo* _typeInfo = nullptr;
    };

    /// Template implementation of the object factory.
    template <class T> class ObjectFactoryImpl : public ObjectFactory
    {
    public:
        /// Construct.
        ObjectFactoryImpl() { _typeInfo = T::GetTypeInfoStatic(); }

        /// Create an object of the specific type.
        SharedPtr<Object> CreateObject() override { return MakeShared<T>(); }
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
