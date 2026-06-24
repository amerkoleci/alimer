// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Property.h"

namespace Alimer
{
    class TypeInfoReflection;
    class ObjectFactory;
    template <class T> class ObjectFactoryImpl;

    /// Type info.
    class ALIMER_API TypeInfo final
    {
    public:
        /// Construct.
        TypeInfo(const char* typeName, const TypeInfo* baseTypeInfo);
        /// Destruct.
        ~TypeInfo() = default;

        /// Get type info by type hash. Return pointer to it or null if does not exist.
        [[nodiscard]] static const TypeInfo* Get(StringId32 typeId);

        /// Get type info by type hash using generic type. Return pointer to it or null if does not exist.
        template <typename T> static const TypeInfo* Get()
        {
            return Get(T::GetTypeStatic());
        }

        /// Check current type is type of specified type.
        [[nodiscard]] bool IsTypeOf(StringId32 type) const;
        /// Check current type is type of specified type.
        [[nodiscard]] bool IsTypeOf(const TypeInfo* typeInfo) const;
        /// Check current type is type of specified class type.
        template <typename T> [[nodiscard]] bool IsTypeOf() const { return IsTypeOf(T::GetTypeInfoStatic()); }

        /// Return type.
        [[nodiscard]] StringId32 GetType() const { return _type; }
        /// Return type name.
        [[nodiscard]] const std::string& GetTypeName() const { return _typeName; }
        /// Return base type info.
        [[nodiscard]] const TypeInfo* GetBaseTypeInfo() const { return _baseTypeInfo; }
        /// Return type info reflection.
        [[nodiscard]] TypeInfoReflection* GetReflection() const { return _reflection; }
        /// Return properties descriptions, or null if none defined.
        [[nodiscard]] const PropertyVector& GetProperties() const;

    private:
        /// Type.
        StringId32 _type;
        /// Type name.
        std::string _typeName;
        /// Base class type info.
        const TypeInfo* _baseTypeInfo;
        /// Reflection
        TypeInfoReflection* _reflection;
    };

    /// Reflection of type info, used for registering properties, serialization and scripting.
    class ALIMER_API TypeInfoReflection final : public RefCounted
    {
    public:
        TypeInfoReflection(const TypeInfo* typeInfo);

        [[nodiscard]] ObjectFactory* GetFactory() const { return _factory.get(); }
        void SetFactory(ObjectFactory* factory);

        /// Register an object factory, template version.
        template <class T> inline void SetFactory()
        {
            SetFactory(new ObjectFactoryImpl<T>());
        }

        [[nodiscard]] const String& GetCategory() const { return _catogory; }
        void SetCategory(StringView value) { _catogory = value; }

        [[nodiscard]] const String& GetDisplayName() const { return _displayName; }
        void SetDisplayName(StringView value) { _displayName = value; }

        [[nodiscard]] const TypeInfo* GetTypeInfo() const { return _typeInfo; }
        [[nodiscard]] const std::string& GetTypeName() const { return _typeInfo ? _typeInfo->GetTypeName() : kEmptyString; }
        [[nodiscard]] StringId32 GetTypeNameId() const { return _typeInfo ? _typeInfo->GetType() : StringId32::Empty; }

        bool RegisterProperty(PropertyInfo* property);

        /// Return properties descriptions, or null if none defined.
        [[nodiscard]] PropertyVector& GetProperties() { return _properties; }

    private:
        const TypeInfo* _typeInfo = nullptr;
        std::unique_ptr<ObjectFactory> _factory = nullptr;
        String _catogory;
        String _displayName;
        PropertyVector _properties;
    };

    /// Base class for object factories.
    class ALIMER_API ObjectFactory
    {
    public:
        /// Destructor.
        virtual ~ObjectFactory() = default;

        /// /// Create an object.
        virtual SharedPtr<Object> CreateObject() = 0;
    };

    /// Template implementation of the object factory.
    template <class T> class ObjectFactoryImpl : public ObjectFactory
    {
    public:
        /// Create an object of the specific type.
        SharedPtr<Object> CreateObject() override { return MakeShared<T>(); }
    };

    /// Get type info reflection by type hash. Return pointer to it or null if does not exist.
    ALIMER_API TypeInfoReflection* GetTypeInfoReflection(StringId32 typeId);

    /// Get type info reflection by type info. Return pointer to it or null if does not exist.
    ALIMER_API TypeInfoReflection* GetTypeInfoReflection(const TypeInfo* typeInfo);

    /// Register an object factory.
    ALIMER_API void RegisterFactory(const TypeInfo* typeInfo, ObjectFactory* factory);
    /// Create an object by type hash. Return pointer to it or null if no factory found.
    ALIMER_API SharedPtr<Object> CreateObject(const TypeInfo* typeInfo);
    /// Create an object by type hash. Return pointer to it or null if no factory found.
    ALIMER_API SharedPtr<Object> CreateObject(StringId32 type);

    /// Register a per-class property. If a property with the same name already exists, it will be replaced.
    ALIMER_API void RegisterProperty(const StringId32& type, PropertyInfo* property);
    /// Copy all base class properties.
    ALIMER_API void CopyBaseProperties(const StringId32& type, const StringId32& baseType);
    /// Copy one base class property.
    ALIMER_API void CopyBaseProperty(const StringId32& type, const StringId32& baseType, const std::string& name);

    /// Register an object factory, template version.
    template <class T> static inline void RegisterFactory()
    {
        RegisterFactory(T::GetTypeInfoStatic(), new ObjectFactoryImpl<T>());
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

    /// Get type info reflection by type hash using generic type. Return pointer to it or null if does not exist.
    template <typename T> static TypeInfoReflection* GetTypeInfoReflection()
    {
        return GetTypeInfoReflection(T::GetTypeStatic());
    }

    /// Register a per-class property, template version. Should not be used for base class properties unless the type is explicitly specified, as by default the property will be re-registered to the base class redundantly.
    template <typename T, typename U>
    static void RegisterProperty(const char* name, U(T::* getFunction)() const, void (T::* setFunction)(U), const U& defaultValue = U())
    {
        RegisterProperty(T::GetTypeStatic(), new PropertyInfoImpl<U>(name, new PropertyAccessorImpl<T, U>(getFunction, setFunction), defaultValue));
    }

    template<typename T, typename TEnum, typename = typename std::enable_if_t<std::is_enum_v<TEnum>>>
    static void RegisterEnumProperty(const char* name, TEnum(T::* getFunction)() const, void (T::* setFunction)(TEnum), const TEnum& defaultValue = TEnum())
    {
        RegisterProperty(T::GetTypeStatic(), new PropertyInfoEnumImpl<TEnum>(name, new PropertyAccessorImpl<T, TEnum>(getFunction, setFunction), defaultValue));
    }

    /// Register a per-class property with reference access, template version. Should not be used for base class properties unless the type is explicitly specified, as by default the property will be re-registered to the base class redundantly.
    template <typename T, typename U> static void RegisterRefProperty(const char* name, const U& (T::* getFunction)() const, void (T::* setFunction)(const U&), const U& defaultValue = U())
    {
        RegisterProperty(T::GetTypeStatic(), new PropertyInfoImpl<U>(name, new RefPropertyAccessorImpl<T, U>(getFunction, setFunction), defaultValue));
    }

    /// Register a per-class attribute with mixed reference access, template version. Should not be used for base class attributes unless the type is explicitly specified, as by default the attribute will be re-registered to the base class redundantly.
    template <class T, class U> static void RegisterMixedRefProperty(const char* name, U(T::* getFunction)() const, void (T::* setFunction)(const U&), const U& defaultValue = U())
    {
        RegisterProperty(T::GetTypeStatic(), new PropertyInfoImpl<U>(name, new MixedRefPropertyAccessorImpl<T, U>(getFunction, setFunction), defaultValue));
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
}
