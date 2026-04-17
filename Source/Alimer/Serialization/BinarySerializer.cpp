// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/IO/Stream.h"
#include "Alimer/Core/Assert.h"
#include "Alimer/Core/Log.h"
#include "Alimer/Serialization/BinarySerializer.h"

namespace Alimer
{
    struct BinarySerializerImpl
    {
        virtual ~BinarySerializerImpl() = default;

        virtual void Serialize(bool& value) = 0;
        virtual void Serialize(int8_t& value) = 0;
        virtual void Serialize(uint8_t& value) = 0;
        virtual void Serialize(int16_t& value) = 0;
        virtual void Serialize(uint16_t& value) = 0;
        virtual void Serialize(int32_t& value) = 0;
        virtual void Serialize(uint32_t& value) = 0;
        virtual void Serialize(int64_t& value) = 0;
        virtual void Serialize(uint64_t& value) = 0;
        virtual void Serialize(float& value) = 0;
        virtual void Serialize(double& value) = 0;
        virtual void Serialize(const char* value) = 0;
        virtual void Serialize(std::string& value) = 0;
    };

    struct ComputeSizeSerializer final : public BinarySerializerImpl
    {
        size_t size{};

        void Serialize(bool&) override
        {
            size += sizeof(bool);
        }

        void Serialize(int8_t&) override
        {
            size += sizeof(int8_t);
        }

        void Serialize(uint8_t&) override
        {
            size += sizeof(uint8_t);
        }

        void Serialize(int16_t&) override
        {
            size += sizeof(int16_t);
        }

        void Serialize(uint16_t&) override
        {
            size += sizeof(uint16_t);
        }

        void Serialize(int32_t&) override
        {
            size += sizeof(int32_t);
        }

        void Serialize(uint32_t&) override
        {
            size += sizeof(uint32_t);
        }

        void Serialize(int64_t&) override
        {
            size += sizeof(int64_t);
        }

        void Serialize(uint64_t&) override
        {
            size += sizeof(uint64_t);
        }

        void Serialize(float&) override
        {
            size += sizeof(float);
        }

        void Serialize(double&) override
        {
            size += sizeof(double);
        }

        void Serialize(const char* value) override
        {
            size += strlen(value);
        }

        void Serialize(std::string& value) override
        {
            size += value.length();
        }
    };

    struct BinarySerializerWriter final : public BinarySerializerImpl
    {
        Stream& stream;

        BinarySerializerWriter(Stream& stream_)
            : stream(stream_)
        {
        }

        ~BinarySerializerWriter()
        {
        }

        void Serialize(bool& value) override
        {
            stream.Write(value);
        }

        void Serialize(int8_t& value) override
        {
            stream.Write(value);
        }

        void Serialize(uint8_t& value) override
        {
            stream.Write(value);
        }

        void Serialize(int16_t& value) override
        {
            stream.Write(value);
        }

        void Serialize(uint16_t& value) override
        {
            stream.Write(value);
        }

        void Serialize(int32_t& value) override
        {
            stream.Write(value);
        }

        void Serialize(uint32_t& value) override
        {
            stream.Write(value);
        }

        void Serialize(int64_t& value) override
        {
            stream.Write(value);
        }

        void Serialize(uint64_t& value) override
        {
            stream.Write(value);
        }

        void Serialize(float& value) override
        {
            stream.Write(value);
        }

        void Serialize(double& value) override
        {
            stream.Write(value);
        }

        void Serialize(const char* value) override
        {
            stream.Write(value);
        }

        void Serialize(std::string& value) override
        {
            stream.Write(value);
        }
    };

    struct BinarySerializerReader final : public BinarySerializerImpl
    {
        Stream& stream;

        BinarySerializerReader(Stream& stream_)
            : stream(stream_)
        {
        }

        ~BinarySerializerReader()
        {
        }

        void Serialize(bool& value) override
        {
            stream.Read(value);
        }

        void Serialize(int8_t& value) override
        {
            stream.Read(value);
        }

        void Serialize(uint8_t& value) override
        {
            stream.Read(value);
        }

        void Serialize(int16_t& value) override
        {
            stream.Read(value);
        }

        void Serialize(uint16_t& value) override
        {
            stream.Read(value);
        }

        void Serialize(int32_t& value) override
        {
            stream.Read(value);
        }

        void Serialize(uint32_t& value) override
        {
            stream.Read(value);
        }

        void Serialize(int64_t& value) override
        {
            stream.Read(value);
        }

        void Serialize(uint64_t& value) override
        {
            stream.Read(value);
        }

        void Serialize(float& value) override
        {
            stream.Read(value);
        }

        void Serialize(double& value) override
        {
            stream.Read(value);
        }

        void Serialize(const char* value) override
        {
            stream.Read(value);
        }

        void Serialize(std::string& value) override
        {
            stream.Write(value);
        }
    };

    BinarySerializer::BinarySerializer()
        : _mode(SerializerMode::Write)
    {
        impl = new ComputeSizeSerializer();
    }

    BinarySerializer::BinarySerializer(Stream& stream, SerializerMode mode)
        : _mode(mode)
    {
        if (stream.CanWrite())
            impl = new BinarySerializerWriter(stream);
        else
            impl = new BinarySerializerReader(stream);
    }

    BinarySerializer::~BinarySerializer()
    {
        delete impl;
        impl = nullptr;
    }

    void BinarySerializer::Serialize(bool& value)
    {
        impl->Serialize(value);
    }

    void BinarySerializer::Serialize(int8_t& value)
    {
        impl->Serialize(value);
    }

    void BinarySerializer::Serialize(uint8_t& value)
    {
        impl->Serialize(value);
    }

    void BinarySerializer::Serialize(int16_t& value)
    {
        impl->Serialize(value);
    }

    void BinarySerializer::Serialize(uint16_t& value)
    {
        impl->Serialize(value);
    }

    void BinarySerializer::Serialize(int32_t& value)
    {
        impl->Serialize(value);
    }

    void BinarySerializer::Serialize(uint32_t& value)
    {
        impl->Serialize(value);
    }

    void BinarySerializer::Serialize(int64_t& value)
    {
        impl->Serialize(value);
    }

    void BinarySerializer::Serialize(uint64_t& value)
    {
        impl->Serialize(value);
    }

    void BinarySerializer::Serialize(float& value)
    {
        impl->Serialize(value);
    }

    void BinarySerializer::Serialize(double& value)
    {
        impl->Serialize(value);
    }

    void BinarySerializer::Serialize(const char* value)
    {
        impl->Serialize(value);
    }

    void BinarySerializer::Serialize(std::string& value)
    {
        impl->Serialize(value);
    }
}

