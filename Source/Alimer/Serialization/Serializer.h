// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Serialization/Types.h"
#include "Alimer/Core/StringId.h"
#include "Alimer/Math/Quaternion.h"
#include "Alimer/Math/Color.h"
#include <stack>

namespace Alimer
{
    class JsonValue;

    class ALIMER_API Serializer
    {
    public:
        virtual ~Serializer() { }

        SerializerMode GetMode() const { return _mode; }
        bool IsReading() const { return _mode == SerializerMode::Read; }
        bool IsWriting() const { return _mode == SerializerMode::Write; }

        virtual void BeginObject(const std::string& key) = 0;
        virtual void EndObject() = 0;
        virtual void BeginArray(const std::string& key, uint32_t& count) = 0;
        virtual void EndArray() = 0;

        virtual void Serialize(std::string_view key, bool& value) = 0;
        virtual void Write(std::string_view key, int8_t value) = 0;
        virtual void Write(std::string_view key, uint8_t value) = 0;
        virtual void Write(std::string_view key, int16_t value) = 0;
        virtual void Write(std::string_view key, uint16_t value) = 0;
        virtual void Write(std::string_view key, int32_t value) = 0;
        virtual void Write(std::string_view key, uint32_t value) = 0;
        virtual void Write(std::string_view key, int64_t value) = 0;
        virtual void Write(std::string_view key, uint64_t value) = 0;
        virtual void Write(std::string_view key, float value) = 0;
        virtual void Write(std::string_view key, double value) = 0;
        //virtual void Serialize(const std::string& value)
        //{
        //    Serialize(kEmptyStringView, value);
        //}

        virtual void Serialize(std::string_view key, const std::string& value) = 0;
        //virtual void Write(std::string_view key, const StringId& value) = 0;
        virtual void Write(std::string_view key, const Vector2& value) = 0;
        virtual void Write(std::string_view key, const Vector3& value) = 0;
        virtual void Write(std::string_view key, const Vector4& value) = 0;
        virtual void Write(std::string_view key, const Quaternion& value) = 0;

    protected:
        Serializer(SerializerMode mode)
            : _mode(mode)
        {

        }

        SerializerMode _mode = SerializerMode::Read;
    };

    /// General purpose serializer which andles both reading and writing via the same interface.
    class ALIMER_API JsonSerializer final : public Serializer
    {
    public:
        JsonSerializer();
        JsonSerializer(std::string_view filePath, SerializerMode mode);
        ~JsonSerializer();

        // Non-copyable and non-movable
        JsonSerializer(const JsonSerializer&) = delete;
        JsonSerializer(const JsonSerializer&&) = delete;
        JsonSerializer& operator=(const JsonSerializer&) = delete;
        JsonSerializer& operator=(const JsonSerializer&&) = delete;

        void BeginObject(const std::string& key) override;
        void EndObject() override;
        void BeginArray(const std::string& key, uint32_t& count) override;
        void EndArray() override;

        void Serialize(std::string_view key, bool& value) override;
        void Write(std::string_view key, int8_t value) override;
        void Write(std::string_view key, uint8_t value) override;
        void Write(std::string_view key, int16_t value) override;
        void Write(std::string_view key, uint16_t value) override;
        void Write(std::string_view key, int32_t value) override;
        void Write(std::string_view key, uint32_t value) override;
        void Write(std::string_view key, int64_t value) override;
        void Write(std::string_view key, uint64_t value) override;
        void Write(std::string_view key, float value) override;
        void Write(std::string_view key, double value) override;
        void Serialize(std::string_view key, const std::string& value) override;
        //void Write(std::string_view key, const StringId& value) override;
        void Write(std::string_view key, const Vector2& value) override;
        void Write(std::string_view key, const Vector3& value) override;
        void Write(std::string_view key, const Vector4& value) override;
        void Write(std::string_view key, const Quaternion& value) override;

        std::string ToString();

    private:
        struct Impl;

        Impl* impl;
    };

#if TODO
    class ALIMER_API JsonDeserializer final : public IDeserializer
    {
        JsonValue data;
        std::stack<JsonValue*> hierarchy;

    public:
        JsonDeserializer(std::string_view json);
        //JsonDeserializer(const vector<uint8>& bson);
        //JsonDeserializer(const fs::path& filePath);

        bool BeginChild(const std::string& name) override;
        void EndChild() override;

        size_t GetArraySize() override;
        bool BeginArrayElement(size_t index) override;
        void EndArrayElement() override;

        bool Read(std::string_view key, bool& value) override;
        bool Read(std::string_view key, int8_t& value) override;
        bool Read(std::string_view key, uint8_t& value) override;
        bool Read(std::string_view key, int16_t& value) override;
        bool Read(std::string_view key, uint16_t& value) override;
        bool Read(std::string_view key, int32_t& value) override;
        bool Read(std::string_view key, uint32_t& value) override;
        bool Read(std::string_view key, int64_t& value) override;
        bool Read(std::string_view key, uint64_t& value) override;
        bool Read(std::string_view key, float& value) override;
        bool Read(std::string_view key, double& value) override;
        bool Read(std::string_view key, std::string& value) override;
        bool Read(std::string_view key, Alimer::UUID& value) override;
        bool Read(std::string_view key, Vector2& value) override;
        bool Read(std::string_view key, Vector3& value) override;
        bool Read(std::string_view key, Vector4& value) override;
        bool Read(std::string_view key, Quaternion& value) override;
        bool Read(std::string_view key, AssetRef& value) override;
        bool Read(std::string_view key, AssetRefList& value) override;
    };
#endif // 0

}
