// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/IO/FileSystem.h"
#include "Alimer/Core/Assert.h"
#include "Alimer/Core/Log.h"
#include "Alimer/Assets/AssetRef.h"
#include "Alimer/Serialization/Serializer.h"
#include "Alimer/Serialization/rapidjson_wrapper.h"
using namespace rapidjson;

namespace Alimer
{
    struct JsonSerializer::Impl
    {
        SerializerMode mode;
        std::string_view filePath;
        Document doc;
        std::stack<Value*> hierarchy;

        Impl(std::string_view filePath_, SerializerMode mode_)
            : filePath(filePath_)
            , mode(mode_)
        {
            if (mode_ == SerializerMode::Read)
            {
                FileStream stream(filePath);
                const size_t dataSize = stream.GetSize() - stream.GetPosition();
                std::unique_ptr<char[]> buffer(new char[dataSize]);
                if (stream.Read(buffer.get(), dataSize) != dataSize)
                    return;

                doc.Parse(buffer.get());
                if (doc.HasParseError())
                {
                    //ALIMER_LOG_ERROR("Failed to parse JSON file: %s, error code: %d, offset: %zu", filePath.data(), doc.GetParseError(), doc.GetErrorOffset());
                    return;
                }
                hierarchy.emplace(&doc);
            }
            else
            {
                doc.SetObject();
            }

            hierarchy.emplace(&doc);
        }
    };

    JsonSerializer::JsonSerializer()
        : JsonSerializer(kEmptyStringView, SerializerMode::Write)
    {

    }

    JsonSerializer::JsonSerializer(std::string_view filePath, SerializerMode mode)
        : Serializer(mode)
        , impl(new Impl(filePath, mode))
    {

    }

    JsonSerializer::~JsonSerializer()
    {
        ALIMER_ASSERT(impl->hierarchy.size() == 1);

        if (!impl->filePath.empty())
        {
            StringBuffer buffer;
            PrettyWriter<StringBuffer> writer(buffer);
            //Writer<StringBuffer> writer(buffer);
            impl->doc.Accept(writer);

            std::string str = buffer.GetString();

            FileStream stream(impl->filePath, FileMode::Write);
            stream.Write(&str[0], str.length());
        }

        delete impl; impl = nullptr;
    }

    void JsonSerializer::BeginObject(const std::string& key)
    {
        if (_mode == SerializerMode::Read)
        {
            Value& parent = *impl->hierarchy.top();

            ALIMER_ASSERT(parent.IsObject());
            ALIMER_ASSERT(parent.HasMember(key.c_str()));
            impl->hierarchy.push(&parent[key.c_str()]);
            return;
        }
        else
        {
            Value child(kObjectType);
            Value& parent = *impl->hierarchy.top();
            parent.AddMember(Value(key.c_str(), impl->doc.GetAllocator()).Move(), child, impl->doc.GetAllocator());
            impl->hierarchy.push(&parent[key.c_str()]);
        }
    }

    void JsonSerializer::EndObject()
    {
        ALIMER_ASSERT(impl->hierarchy.size() > 1); // No child to end.

        impl->hierarchy.pop();
    }

    void JsonSerializer::BeginArray(const std::string& key, uint32_t& count)
    {
        if (_mode == SerializerMode::Read)
        {
            Value& parent = *impl->hierarchy.top();
            ALIMER_ASSERT(parent.IsObject());
            ALIMER_ASSERT(parent.HasMember(key.c_str()));
            impl->hierarchy.push(&parent[key.c_str()]);
            count = static_cast<uint32_t>(impl->hierarchy.top()->Size());
        }
        else
        {
            Value child(kArrayType);
            Value& parent = *impl->hierarchy.top();
            parent.AddMember(StringRef(key.data()), child, impl->doc.GetAllocator());
            impl->hierarchy.push(&child);
        }
    }

    void JsonSerializer::EndArray()
    {
        ALIMER_ASSERT(impl->hierarchy.size() > 1); // No child to end.
        impl->hierarchy.pop();
    }

    void JsonSerializer::Serialize(std::string_view key, bool& value)
    {
        Value& parent = *impl->hierarchy.top();
        parent.AddMember(StringRef(key.data()), value, impl->doc.GetAllocator());
    }


    void JsonSerializer::Write(std::string_view key, int8_t value)
    {
        auto& parent = *impl->hierarchy.top();
        parent.AddMember(StringRef(key.data()), value, impl->doc.GetAllocator());
    }

    void JsonSerializer::Write(std::string_view key, uint8_t value)
    {
        auto& parent = *impl->hierarchy.top();
        parent.AddMember(StringRef(key.data()), value, impl->doc.GetAllocator());
    }

    void JsonSerializer::Write(std::string_view key, int16_t value)
    {
        auto& parent = *impl->hierarchy.top();
        parent.AddMember(StringRef(key.data()), value, impl->doc.GetAllocator());
    }

    void JsonSerializer::Write(std::string_view key, uint16_t value)
    {
        auto& parent = *impl->hierarchy.top();
        parent.AddMember(StringRef(key.data()), value, impl->doc.GetAllocator());
    }

    void JsonSerializer::Write(std::string_view key, int32_t value)
    {
        auto& parent = *impl->hierarchy.top();
        parent.AddMember(StringRef(key.data()), value, impl->doc.GetAllocator());
    }

    void JsonSerializer::Write(std::string_view key, uint32_t value)
    {
        auto& parent = *impl->hierarchy.top();
        parent.AddMember(StringRef(key.data()), value, impl->doc.GetAllocator());
    }

    void JsonSerializer::Write(std::string_view key, int64_t value)
    {
        auto& parent = *impl->hierarchy.top();
        parent.AddMember(StringRef(key.data()), value, impl->doc.GetAllocator());
    }

    void JsonSerializer::Write(std::string_view key, uint64_t value)
    {
        auto& parent = *impl->hierarchy.top();
        parent.AddMember(StringRef(key.data()), value, impl->doc.GetAllocator());
    }

    void JsonSerializer::Write(std::string_view key, float value)
    {
        //auto& object = *hierarchy.top();
        //if (key.empty())
        //    object.Push(value);
        //else
        //    object.Set(key, value);
    }

    void JsonSerializer::Write(std::string_view key, double value)
    {
        auto& parent = *impl->hierarchy.top();
        parent.AddMember(StringRef(key.data()), value, impl->doc.GetAllocator());
    }

    void JsonSerializer::Serialize(std::string_view key, const std::string& value)
    {
        if (_mode == SerializerMode::Read)
        {
            Value& parent = *impl->hierarchy.top();

            ALIMER_ASSERT(parent.IsObject());
            //ALIMER_ASSERT(parent.HasMember(key.c_str()));
            //const Value& child = parent[key.c_str()];
            //ALIMER_ASSERT(child.IsString());
            //value = child.GetString();
        }
        else
        {
            auto& parent = *impl->hierarchy.top();
            if (key.empty())
            {
                parent.PushBack(StringRef(value.c_str()), impl->doc.GetAllocator());
            }
            else
            {
                parent.AddMember(StringRef(key.data()), StringRef(value.c_str()), impl->doc.GetAllocator());
            }
        }
    }

    //void JsonSerializer::Write(std::string_view key, const Alimer::UUID& value)
    //{
    //    uint64_t writeValue = static_cast<uint64_t>(value);
    //    Write(key, writeValue);
    //}

    void JsonSerializer::Write(std::string_view key, const Vector2& value)
    {
        //auto& object = *hierarchy.top();
        //JsonValue jsonValue;
        //jsonValue.Push(value.x);
        //jsonValue.Push(value.y);
        //object.Set(key, jsonValue);
    }

    void JsonSerializer::Write(std::string_view key, const Vector3& value)
    {
        //auto& object = *hierarchy.top();
        //JsonValue jsonValue;
        //jsonValue.Push(value.x);
        //jsonValue.Push(value.y);
        //jsonValue.Push(value.z);
        //object.Set(key, jsonValue);
    }

    void JsonSerializer::Write(std::string_view key, const Vector4& value)
    {
        //auto& object = *hierarchy.top();
        //JsonValue jsonValue;
        //jsonValue.Push(value.x);
        //jsonValue.Push(value.y);
        //jsonValue.Push(value.z);
        //jsonValue.Push(value.w);
        //object.Set(key, jsonValue);
    }

    void JsonSerializer::Write(std::string_view key, const Quaternion& value)
    {
        //auto& object = *hierarchy.top();
        //JsonValue jsonValue;
        //jsonValue.Push(value.x);
        //jsonValue.Push(value.y);
        //jsonValue.Push(value.z);
        //jsonValue.Push(value.w);
        //object.Set(key, jsonValue);
    }

    std::string JsonSerializer::ToString()
    {
        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer);
        //Writer<StringBuffer> writer(buffer);
        impl->doc.Accept(writer);

        std::string result = buffer.GetString();
        return result;
    }
}

