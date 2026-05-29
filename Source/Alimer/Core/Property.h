// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Variant.h"
#include "Alimer/Core/Assert.h"
#include "Alimer/IO/Stream.h"

namespace Alimer
{
    //class Serializer;
    //class IDeserializer;
    class PropertyInfo;
    class Object;

    using PropertyVector = Vector<SharedPtr<PropertyInfo>>;

    /// Helper class for accessing serializable variables via getter and setter functions.
    class ALIMER_API PropertyAccessor
    {
    public:
        /// Destruct.
        virtual ~PropertyAccessor() = default;

        /// Get the current value of the variable.
        virtual void Get(const Object* instance, void* dest) const = 0;
        /// Set new value for the variable.
        virtual void Set(Object* instance, const void* source) = 0;
    };

    /// Description of an automatically serializable properties.
    class ALIMER_API PropertyInfo : public RefCounted
    {
    public:
        /// Construct.
        PropertyInfo(const char* name, PropertyAccessor* accessor, const char** enumNames = nullptr);
        ~PropertyInfo() override;

        /// Copy to a value in memory.
        void GetValue(const Object* instance, void* dest) const;

        /// Set from a value in memory.
        void SetValue(Object* instance, const void* source);

        /// Return whether is default value.
        virtual bool IsDefault(const Object* instance) const = 0;

        /// Return property name.
        const std::string& GetName() const { return name; }

        /// Return property type.
        virtual VariantType GetPropertyType() const = 0;

        /// Serialize.
        //virtual void Serialize(Object* instance, Serializer& serializer) = 0;

        /// Deserialize.
        //virtual void Deserialize(Object* instance, IDeserializer& deserializer) = 0;

        /// Deserialize from a binary stream.
        virtual void FromBinary(Object* instance, Stream& source) = 0;
        /// Serialize to a binary stream.
        virtual void ToBinary(Object* instance, Stream& dest) = 0;

        /// Skip binary data of an attribute.
        static void Skip(VariantType type, Stream& source);

        /// Serialize property value.
        //static void Serialize(Serializer& serializer, const std::string& propertyName, VariantType type, const void* source);

        /// Deserialize property value.
        //static bool Deserialize(IDeserializer& deserializer, const std::string& propertyName, VariantType type, void* dest);

    protected:
        /// Variable name.
        std::string name;
        /// Attribute accessor.
        std::unique_ptr<PropertyAccessor> accessor;
        /// Enum names.
        const char** enumNames;
    };

    /// Template implementation of an attribute description with specific type.
    template <class T> class PropertyInfoImpl final : public PropertyInfo
    {
    public:
        /// Construct.
        PropertyInfoImpl(const char* name, PropertyAccessor* accessor_, const T& defaultValue, const char** enumNames = nullptr)
            : PropertyInfo(name, accessor_, enumNames)
            , _defaultValue(defaultValue)
        {
        }

        /// Copy current attribute value.
        void GetValue(const Object* instance, T& dest) const { accessor->Get(instance, &dest); }

        /// Set new attribute value.
        void SetValue(Object* instance, const T& source) { accessor->Set(instance, &source); }

        /// Return current attribute value.
        T GetValue(const Object* instance) const
        {
            T result;
            accessor->Get(instance, &result);
            return result;
        }

        /// Return default value.
        const T& GetDefaultValue() const { return _defaultValue; }

        /// Return whether is default value.
        bool IsDefault(const Object* instance) const override { return GetValue(instance) == _defaultValue; }

        VariantType GetPropertyType() const override
        {
            return GetVariantType<T>();
        }

#if TODO_SERIALIAZION
        void Serialize(Object* instance, Serializer& serializer) override
        {
            T value;
            accessor->Get(instance, &value);
            PropertyInfo::Serialize(serializer, name, GetPropertyType(), &value);
        }

        void Deserialize(Serializable* instance, IDeserializer& deserializer) override
        {
            T value;
            if (PropertyInfo::Deserialize(deserializer, name, GetPropertyType(), &value))
            {
                accessor->Set(instance, &value);
            }
        }
#endif // TODO_SERIALIAZION

        /// Deserialize from a binary stream.
        void FromBinary(Object* instance, Stream& source) override
        {
            T value;
            source.Read(&value, sizeof(T));
            accessor->Set(instance, &value);
        }

        /// Serialize to a binary stream.
        void ToBinary(Object* instance, Stream& dest) override
        {
            T value;
            accessor->Get(instance, &value);
            dest.Write(&value, sizeof(T));
        }

    private:
        /// Default value.
        T _defaultValue;
    };

    /// Template implementation for accessing serializable variables.
    template <class T, class U> class PropertyAccessorImpl final : public PropertyAccessor
    {
    public:
        typedef U(T::* GetFunctionPtr)() const;
        typedef void (T::* SetFunctionPtr)(U);

        /// Construct with function pointers.
        PropertyAccessorImpl(GetFunctionPtr getPtr, SetFunctionPtr setPtr)
            : get(getPtr)
            , set(setPtr)
        {
            ALIMER_ASSERT(get != nullptr);
            ALIMER_ASSERT(set != nullptr);
        }

        /// Get current value of the variable.
        void Get(const Object* instance, void* dest) const override
        {
            ALIMER_ASSERT(instance);

            U& value = *(reinterpret_cast<U*>(dest));
            const T* classInstance = static_cast<const T*>(instance);
            value = (classInstance->*get)();
        }

        /// Set new value for the variable.
        void Set(Object* instance, const void* source) override
        {
            ALIMER_ASSERT(instance);

            const U& value = *(reinterpret_cast<const U*>(source));
            T* classInstance = static_cast<T*>(instance);
            (classInstance->*set)(value);
        }

    private:
        /// Getter function pointer.
        GetFunctionPtr get;
        /// Setter function pointer.
        SetFunctionPtr set;
    };

    /// Template implementation for accessing serializable variables via functions that use references.
    template <class T, class U> class RefPropertyAccessorImpl final : public PropertyAccessor
    {
    public:
        typedef const U& (T::* GetFunctionPtr)() const;
        typedef void (T::* SetFunctionPtr)(const U&);

        /// Set new value for the variable.
        RefPropertyAccessorImpl(GetFunctionPtr getPtr, SetFunctionPtr setPtr)
            : _get(getPtr)
            , _set(setPtr)
        {
            ALIMER_ASSERT(_get);
            ALIMER_ASSERT(_set);
        }

        /// Get current value of the variable.
        void Get(const Object* instance, void* dest) const override
        {
            ALIMER_ASSERT(instance);

            U& value = *(reinterpret_cast<U*>(dest));
            const T* classPtr = static_cast<const T*>(instance);
            value = (classPtr->*_get)();
        }

        /// Set new value for the variable.
        void Set(Object* instance, const void* source) override
        {
            ALIMER_ASSERT(instance);

            const U& value = *(reinterpret_cast<const U*>(source));
            T* classPtr = static_cast<T*>(instance);
            (classPtr->*_set)(value);
        }

    private:
        /// Getter function pointer.
        GetFunctionPtr _get;
        /// Setter function pointer.
        SetFunctionPtr _set;
    };

    /// Template implementation for accessing serializable variables via functions where the setter uses reference, but the getter does not.
    template <class T, class U> class MixedRefPropertyAccessorImpl final : public PropertyAccessor
    {
    public:
        typedef U(T::* GetFunctionPtr)() const;
        typedef void (T::* SetFunctionPtr)(const U&);

        /// Construct with function pointers.
        MixedRefPropertyAccessorImpl(GetFunctionPtr getPtr, SetFunctionPtr setPtr)
            : _get(getPtr)
            , _set(setPtr)
        {
            ALIMER_ASSERT(_get);
            ALIMER_ASSERT(_set);
        }

        /// Get current value of the variable.
        void Get(const Object* instance, void* dest) const override
        {
            ALIMER_ASSERT(instance);

            U& value = *(reinterpret_cast<U*>(dest));
            const T* classPtr = static_cast<const T*>(instance);
            value = (classPtr->*_get)();
        }

        /// Set new value for the variable.
        void Set(Object* instance, const void* source) override
        {
            ALIMER_ASSERT(instance);

            const U& value = *(reinterpret_cast<const U*>(source));
            T* classPtr = static_cast<T*>(instance);
            (classPtr->*_set)(value);
        }

    private:
        /// Getter function pointer.
        GetFunctionPtr _get;
        /// Setter function pointer.
        SetFunctionPtr _set;
    };
}
