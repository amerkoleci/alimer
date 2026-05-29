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

        [[nodiscard]] const std::string& GetCategory() const { return _catogory; }
        void SetCategory(std::string_view category) { _catogory = category; }

        [[nodiscard]] const TypeInfo* GetTypeInfo() const { return _typeInfo; }
        [[nodiscard]] const std::string& GetTypeName() const { return _typeInfo ? _typeInfo->GetTypeName() : kEmptyString; }
        [[nodiscard]] StringId32 GetTypeNameId() const { return _typeInfo ? _typeInfo->GetType() : StringId32::Empty; }

        bool RegisterProperty(PropertyInfo* property);

        /// Return properties descriptions, or null if none defined.
        [[nodiscard]] PropertyVector& GetProperties() { return _properties; }

    private:
        const TypeInfo* _typeInfo = nullptr;
        std::unique_ptr<ObjectFactory> _factory = nullptr;
        std::string _catogory;
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

    /// Get type info reflection by type hash using generic type. Return pointer to it or null if does not exist.
    template <typename T> static TypeInfoReflection* GetTypeInfoReflection()
    {
        return GetTypeInfoReflection(T::GetTypeStatic());
    }
}
