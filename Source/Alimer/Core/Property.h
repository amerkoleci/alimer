// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Variant.h"
#include "Alimer/Core/Assert.h"
#include "Alimer/IO/Stream.h"

namespace Alimer
{
    class SerializeValue;
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

    /// Description of an automatically serializable properties.
    class ALIMER_API PropertyInfo : public RefCounted
    {
    public:
        /// Construct.
        PropertyInfo(const char* name, PropertyAccessor* accessor);

        /// Copy to a value in memory.
        void GetValue(const Object* instance, void* dest) const;

        /// Set from a value in memory.
        void SetValue(Object* instance, const void* value);

        /// Return property name.
        [[nodiscard]] const std::string& GetName() const { return _name; }

        /// Return whether is default value.
        [[nodiscard]] virtual bool IsDefault(const Object* instance) const = 0;

        /// Return property type.
        [[nodiscard]] virtual VariantType GetPropertyType() const = 0;

        /// Serialize from serialize value.
        virtual void ToSerializeValue(Object* instance, SerializeValue& dest) = 0;

        /// Deserialize from a serialize value.
        virtual void FromSerializeValue(Object* instance, const SerializeValue& source) = 0;

        /// Deserialize from a binary stream.
        virtual void FromBinary(Object* instance, Stream& source) = 0;
        /// Serialize to a binary stream.
        virtual void ToBinary(Object* instance, Stream& dest) = 0;

        /// Skip binary data of an attribute.
        static void Skip(VariantType type, Stream& source);

        /// Serialize property value.
        static void ToSerializeValue(VariantType type, SerializeValue& dest, const void* source);

        /// Deserialize property value.
        static void FromSerializeValue(VariantType type, void* dest, const SerializeValue& source);

    protected:
        /// Variable name.
        std::string _name;
        /// Attribute accessor.
        std::unique_ptr<PropertyAccessor> _accessor;
    };

    /// Template implementation of an attribute description with specific type.
    template <class T> class PropertyInfoImpl final : public PropertyInfo
    {
    public:
        /// Construct.
        PropertyInfoImpl(const char* name, PropertyAccessor* accessor_, const T& defaultValue)
            : PropertyInfo(name, accessor_)
            , _defaultValue(defaultValue)
        {
        }

        /// Copy current attribute value.
        void GetValue(const Object* instance, T& dest) const { _accessor->Get(instance, &dest); }

        /// Set new attribute value.
        void SetValue(Object* instance, const T& source) { _accessor->Set(instance, &source); }

        /// Return current attribute value.
        [[nodiscard]] T GetValue(const Object* instance) const
        {
            T result;
            _accessor->Get(instance, &result);
            return result;
        }

        /// Return default value.
        [[nodiscard]] const T& GetDefaultValue() const { return _defaultValue; }

        /// Return whether is default value.
        [[nodiscard]] bool IsDefault(const Object* instance) const override { return GetValue(instance) == _defaultValue; }

        VariantType GetPropertyType() const override
        {
            return GetVariantType<T>();
        }

        void ToSerializeValue(Object* instance, SerializeValue& dest) override
        {
            T value;
            _accessor->Get(instance, &value);
            PropertyInfo::ToSerializeValue(GetPropertyType(), dest, &value);
        }

        void FromSerializeValue(Object* instance, const SerializeValue& source) override
        {
            T value;
            PropertyInfo::FromSerializeValue(GetPropertyType(), &value, source);
            _accessor->Set(instance, &value);
        }

        /// Deserialize from a binary stream.
        void FromBinary(Object* instance, Stream& source) override
        {
            T value;
            source.Read(&value, sizeof(T));
            _accessor->Set(instance, &value);
        }

        /// Serialize to a binary stream.
        void ToBinary(Object* instance, Stream& dest) override
        {
            T value;
            _accessor->Get(instance, &value);
            dest.Write(&value, sizeof(T));
        }

    private:
        /// Default value.
        T _defaultValue;
    };

    /// Template implementation of an attribute description with specific type.
    template <class T> class PropertyInfoEnumImpl final : public PropertyInfo
    {
    public:
        /// Construct.
        PropertyInfoEnumImpl(const char* name, PropertyAccessor* accessor_, const T& defaultValue)
            : PropertyInfo(name, accessor_)
            , _defaultValue(defaultValue)
        {}

        /// Copy current attribute value.
        void GetValue(const Object* instance, T& dest) const { _accessor->Get(instance, &dest); }

        /// Set new attribute value.
        void SetValue(Object* instance, const T& source) { _accessor->Set(instance, &source); }

        /// Return current attribute value.
        [[nodiscard]] T GetValue(const Object* instance) const
        {
            T result;
            _accessor->Get(instance, &result);
            return result;
        }

        /// Return default value.
        [[nodiscard]] const T& GetDefaultValue() const { return _defaultValue; }

        /// Return whether is default value.
        [[nodiscard]] bool IsDefault(const Object* instance) const override { return GetValue(instance) == _defaultValue; }

        VariantType GetPropertyType() const override
        {
            return VariantType::Enum;
        }

        void ToSerializeValue(Object* instance, SerializeValue& dest) override
        {
            T value;
            _accessor->Get(instance, &value);
            PropertyInfo::ToSerializeValue(GetPropertyType(), dest, &value);
        }

        void FromSerializeValue(Object* instance, const SerializeValue& source) override
        {
            T value;
            PropertyInfo::FromSerializeValue(GetPropertyType(), &value, source);
            _accessor->Set(instance, &value);
        }

        /// Deserialize from a binary stream.
        void FromBinary(Object* instance, Stream& source) override
        {
            T value;
            source.Read(&value, sizeof(T));
            _accessor->Set(instance, &value);
        }

        /// Serialize to a binary stream.
        void ToBinary(Object* instance, Stream& dest) override
        {
            T value;
            _accessor->Get(instance, &value);
            dest.Write(&value, sizeof(T));
        }

    private:
        /// Default value.
        T _defaultValue;
    };
}
