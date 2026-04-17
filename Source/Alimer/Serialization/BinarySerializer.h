// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Serialization/Types.h"
//#include "Alimer/Core/UUID.h"
#include "Alimer/Math/Quaternion.h"
#include "Alimer/Math/Color.h"

namespace Alimer
{
    class Stream;

    /// General purpose serializer which andles both reading and writing from binary file via the same interface.
    class ALIMER_API BinarySerializer final
    {
    public:
        BinarySerializer();
        BinarySerializer(Stream& stream, SerializerMode mode);
        ~BinarySerializer();

        // Non-copyable and non-movable
        BinarySerializer(const BinarySerializer&) = delete;
        BinarySerializer(const BinarySerializer&&) = delete;
        BinarySerializer& operator=(const BinarySerializer&) = delete;
        BinarySerializer& operator=(const BinarySerializer&&) = delete;

        SerializerMode GetMode() const { return _mode; }
        bool IsReading() const { return _mode == SerializerMode::Read; }
        bool IsWriting() const { return _mode == SerializerMode::Write; }
        explicit operator bool() const { return impl != nullptr; }

        void Serialize(bool& value);
        void Serialize(int8_t& value);
        void Serialize(uint8_t& value);
        void Serialize(int16_t& value);
        void Serialize(uint16_t& value);
        void Serialize(int32_t& value);
        void Serialize(uint32_t& value);
        void Serialize(int64_t& value);
        void Serialize(uint64_t& value);
        void Serialize(float& value);
        void Serialize(double& value);
        void Serialize(const char* value);
        void Serialize(std::string& value);

        /// Enum serialization.
        template<typename TEnum, typename = typename std::enable_if<std::is_enum<TEnum>::value>::type>
        void Serialize(TEnum& value)
        {
            Serialize(static_cast<typename std::underlying_type<TEnum>::type>(value));
        }

    private:
        struct BinarySerializerImpl* impl = nullptr;

        SerializerMode _mode = SerializerMode::Read;
    };

    // Forward declares
    template<typename TSerializer, typename TString>
    void Serialize(TSerializer& serializer, const char* key, std::basic_string<TString>& str);

    // Specialized serializers
    template<typename TSerializer>
    void Serialize(TSerializer& serializer, const char* key, bool& val)
    {
        serializer.Serialize(key, val);
    }

    template<typename TSerializer>
    void Serialize(TSerializer& serializer, const char* key, int8_t& val)
    {
        serializer.Serialize(key, val);
    }

    template<typename TSerializer>
    void Serialize(TSerializer& serializer, const char* key, uint8_t& val)
    {
        serializer.Serialize(key, val);
    }

    template<typename TSerializer>
    void Serialize(TSerializer& serializer, const char* key, int16_t& val)
    {
        serializer.Serialize(key, val);
    }

    template<typename TSerializer>
    void Serialize(TSerializer& serializer, const char* key, uint16_t& val)
    {
        serializer.Serialize(key, val);
    }

    template<typename TSerializer>
    void Serialize(TSerializer& serializer, const char* key, int32_t& val)
    {
        serializer.Serialize(key, val);
    }

    template<typename TSerializer>
    void Serialize(TSerializer& serializer, const char* key, uint32_t& val)
    {
        serializer.Serialize(key, val);
    }

    template<typename TSerializer>
    void Serialize(TSerializer& serializer, const char* key, int64_t& val)
    {
        serializer.Serialize(key, val);
    }

    template<typename TSerializer>
    void Serialize(TSerializer& serializer, const char* key, uint64_t& value)
    {
        serializer.Serialize(key, value);
    }

    template<typename TSerializer, typename TString>
    void Serialize(TSerializer& serializer, const char* key, const char* value)
    {
        serializer.Serialize(key, value);
    }

    template<typename TSerializer, typename TString>
    void Serialize(TSerializer& serializer, const char* key, std::basic_string<TString>& value)
    {
        serializer.Serialize(key, value);
    }

    //template<typename TSerializer>
    //void Serialize(TSerializer& serializer, const char* key, Alimer::UUID& uuid)
    //{
    //    serializer.Serialize(key, static_cast<uint64_t>(uuid));
    //}

    template<typename TSerializer>
    void Serialize(TSerializer& serializer, const char* key, Alimer::Vector2& val)
    {
        if (auto object = serializer.Object(key, true))
        {
            serializer.Serialize(nullptr, val.x);
            serializer.Serialize(nullptr, val.y);
        }
    }

    template<typename TSerializer>
    void Serialize(TSerializer& serializer, const char* key, Alimer::Vector3& val)
    {
        if (auto object = serializer.Object(key, true))
        {
            serializer.Serialize(nullptr, val.x);
            serializer.Serialize(nullptr, val.y);
            serializer.Serialize(nullptr, val.z);
        }
    }

    // Trampoline functions
    template<typename TSerializer, typename TValue>
    void Serialize(TSerializer& serializer, const char* key, TValue& val)
    {
        val.Serialize(key, serializer);
    }

#define SERIALIZE_MEMBER(_name) Serialize(serializer, #_name, _name)
}
