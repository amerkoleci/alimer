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
    struct JsonSerializerImpl
    {
        virtual ~JsonSerializerImpl() = default;
        virtual std::string ToString() const = 0;

        virtual void BeginObject(const char* key, bool isArray) = 0;
        virtual void EndObject() = 0;

        virtual bool Serialize(const char* key, bool& value) = 0;
        virtual bool Serialize(const char* key, int8_t& value) = 0;
        virtual bool Serialize(const char* key, uint8_t& value) = 0;
        virtual bool Serialize(const char* key, int16_t& value) = 0;
        virtual bool Serialize(const char* key, uint16_t& value) = 0;
        virtual bool Serialize(const char* key, int32_t& value) = 0;
        virtual bool Serialize(const char* key, uint32_t& value) = 0;
        virtual bool Serialize(const char* key, int64_t& value) = 0;
        virtual bool Serialize(const char* key, uint64_t& value) = 0;
        virtual bool Serialize(const char* key, float& value) = 0;
        virtual bool Serialize(const char* key, double& value) = 0;
        virtual bool Serialize(const char* key, const std::string& value) = 0;

        virtual bool Serialize(const char* key, const Vector2& value) = 0;
    };

    struct JsonWriteSerializer final : public JsonSerializerImpl
    {
        std::string_view filePath;
        Document root;
        std::vector<Value*> hierarchy;
        mutable std::string jsonString;

        JsonWriteSerializer(std::string_view filePath_)
            : filePath(filePath_)
        {
            root.SetObject();
            hierarchy.push_back(&root);
        }

        ~JsonWriteSerializer() override
        {
            ALIMER_ASSERT(hierarchy.size() == 1);

            if (!filePath.empty())
            {
                const std::string& writeJson = ToString();

                FileStream stream(filePath, FileMode::Write);
                stream.Write(writeJson.c_str(), writeJson.length());
            }
        }

        std::string ToString() const override
        {
            StringBuffer buffer;
            PrettyWriter<StringBuffer> writer(buffer);
            //Writer<StringBuffer> writer(buffer);
            root.Accept(writer);
            jsonString.assign(buffer.GetString());
            return jsonString;
        }

        rapidjson::Value& GetObject(int32_t index = -1)
        {
            if (index < 0)
                return *hierarchy.back();

            return *hierarchy.at(index);
        }

        void BeginObject(const char* key, bool isArray) override
        {
            auto& object = GetObject();

            if (object.IsArray())
            {
                if (isArray)
                {
                    object.PushBack(rapidjson::Value(kArrayType), root.GetAllocator());
                    hierarchy.push_back(&object[object.Size() - 1]);
                }
                else
                {
                    object.PushBack(rapidjson::Value(kObjectType), root.GetAllocator());
                    hierarchy.push_back(&object[object.Size() - 1]);
                }
            }
            else
            {
                if (isArray)
                {
                    object.AddMember(StringRef(key), rapidjson::Value(kArrayType), root.GetAllocator());
                    hierarchy.push_back(&object[key]);
                }
                else
                {
                    object.AddMember(StringRef(key), rapidjson::Value(kObjectType), root.GetAllocator());
                    hierarchy.push_back(&object[key]);
                }
            }
        }

        void EndObject() override
        {
            ALIMER_ASSERT(hierarchy.size() > 1); // No child to end.

            hierarchy.pop_back();
        }

        bool Serialize(const char* key, bool& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, int8_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, uint8_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, int16_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, uint16_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }
        bool Serialize(const char* key, int32_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }
        bool Serialize(const char* key, uint32_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }
        bool Serialize(const char* key, int64_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, uint64_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, float& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, double& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, const std::string& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), StringRef(value.c_str()), root.GetAllocator());
            }
            else
            {
                object.PushBack(StringRef(value.c_str()), root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, const Vector2& value) override
        {
            Value& object = GetObject();
            Value jsonValue(kArrayType);
            jsonValue.PushBack(value.x, root.GetAllocator());
            jsonValue.PushBack(value.y, root.GetAllocator());
            if (key)
            {
                object.AddMember(StringRef(key), jsonValue, root.GetAllocator());
            }
            else
            {
                object.PushBack(jsonValue, root.GetAllocator());
            }
            return true;
        }
    };


    struct JsonReadSerializer final : public JsonSerializerImpl
    {
        Document root;
        Vector<Value*> hierarchy;
        Vector<int32_t> vectorStack;

        JsonReadSerializer(const std::string& json)
        {
            root.Parse(json);
            if (root.HasParseError())
            {
                //ALIMER_LOG_ERROR("Failed to parse JSON file: %s, error code: %d, offset: %zu", filePath.data(), doc.GetParseError(), doc.GetErrorOffset());
                return;
            }

            hierarchy.push_back(&root);
        }

        ~JsonReadSerializer() override
        {
            ALIMER_ASSERT(hierarchy.size() == 1);
        }

        std::string ToString() const override
        {
            return kEmptyString;
        }

        rapidjson::Value& GetObject()
        {
            return *hierarchy.back();
        }

        rapidjson::Value& GetObjectWithKey(const char* key)
        {
            if (key)
            {
                return GetObject()[key];
            }
            else
            {
                int32_t idx = vectorStack.back()++;
                return GetObject()[idx];
            }
        }

        void BeginObject(const char* key, bool isArray) override
        {
            auto& object = GetObjectWithKey(key);

            if (object.IsObject())
            {
                hierarchy.push_back(&object);
                //return object.Size();
            }
            else if (object.IsArray())
            {
                hierarchy.push_back(&object);
                vectorStack.push_back(0);
                //return object.size();
            }
        }

        void EndObject() override
        {
            if (hierarchy.back()->IsArray())
                vectorStack.pop_back();
            hierarchy.pop_back();

            ALIMER_ASSERT(hierarchy.size() > 0);
        }

        bool Serialize(const char* key, bool& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, int8_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, uint8_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, int16_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, uint16_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }
        bool Serialize(const char* key, int32_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }
        bool Serialize(const char* key, uint32_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }
        bool Serialize(const char* key, int64_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, uint64_t& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, float& value) override
        {
            Value& object = GetObjectWithKey(key);
            if (object.IsDouble())
            {
                value = object.GetFloat();
                return true;
            }

            return true;
        }

        bool Serialize(const char* key, double& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), value, root.GetAllocator());
            }
            else
            {
                object.PushBack(value, root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, const std::string& value) override
        {
            Value& object = GetObject();
            if (key)
            {
                object.AddMember(StringRef(key), StringRef(value.c_str()), root.GetAllocator());
            }
            else
            {
                object.PushBack(StringRef(value.c_str()), root.GetAllocator());
            }

            return true;
        }

        bool Serialize(const char* key, const Vector2& value) override
        {
            Value& object = GetObject();
            Value jsonValue(kArrayType);
            jsonValue.PushBack(value.x, root.GetAllocator());
            jsonValue.PushBack(value.y, root.GetAllocator());
            if (key)
            {
                object.AddMember(StringRef(key), jsonValue, root.GetAllocator());
            }
            else
            {
                object.PushBack(jsonValue, root.GetAllocator());
            }
            return true;
        }
    };

    JsonSerializer::JsonSerializer()
        : JsonSerializer(kEmptyStringView, SerializerMode::Write)
    {

    }

    JsonSerializer::JsonSerializer(std::string_view filePath, SerializerMode mode)
        : Serializer(mode)
    {
        if (mode == SerializerMode::Write)
        {
            impl = new JsonWriteSerializer(filePath);
        }
        else
        {
            FileStream stream(filePath);
            const size_t dataSize = stream.GetSize() - stream.GetPosition();
            std::unique_ptr<char[]> buffer(new char[dataSize]);
            if (stream.Read(buffer.get(), dataSize) != dataSize)
                return;

            impl = new JsonReadSerializer(std::string(buffer.get(), dataSize));
        }
    }

    JsonSerializer::JsonSerializer(const std::string& json)
        : Serializer(SerializerMode::Read)
    {
        impl = new JsonReadSerializer(json);
    }

    JsonSerializer::~JsonSerializer()
    {
        delete impl; impl = nullptr;
    }

    void JsonSerializer::BeginObject(const char* key, bool isArray)
    {
        impl->BeginObject(key, isArray);
    }

    void JsonSerializer::EndObject()
    {
        impl->EndObject();
    }

    bool JsonSerializer::Serialize(const char* key, bool& value)
    {
        return impl->Serialize(key, value);
    }

    bool JsonSerializer::Serialize(const char* key, int8_t& value)
    {
        return impl->Serialize(key, value);
    }

    bool JsonSerializer::Serialize(const char* key, uint8_t& value)
    {
        return impl->Serialize(key, value);
    }

    bool JsonSerializer::Serialize(const char* key, int16_t& value)
    {
        return impl->Serialize(key, value);
    }

    bool JsonSerializer::Serialize(const char* key, uint16_t& value)
    {
        return impl->Serialize(key, value);
    }

    bool JsonSerializer::Serialize(const char* key, int32_t& value)
    {
        return impl->Serialize(key, value);
    }

    bool JsonSerializer::Serialize(const char* key, uint32_t& value)
    {
        return impl->Serialize(key, value);
    }

    bool JsonSerializer::Serialize(const char* key, int64_t& value)
    {
        return impl->Serialize(key, value);
    }

    bool JsonSerializer::Serialize(const char* key, uint64_t& value)
    {
        return impl->Serialize(key, value);
    }

    bool JsonSerializer::Serialize(const char* key, float& value)
    {
        return impl->Serialize(key, value);
    }

    bool JsonSerializer::Serialize(const char* key, double& value)
    {
        return impl->Serialize(key, value);
    }

    bool JsonSerializer::Serialize(const char* key, const std::string& value)
    {
        return impl->Serialize(key, value);
    }

    //void JsonSerializer::Write(std::string_view key, const Alimer::UUID& value)
    //{
    //    uint64_t writeValue = static_cast<uint64_t>(value);
    //    Write(key, writeValue);
    //}

    bool JsonSerializer::Serialize(const char* key, const Vector2& value)
    {
        return impl->Serialize(key, value);
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

    std::string JsonSerializer::ToString() const
    {
        if (_mode == SerializerMode::Read)
        {
            return {};
        }

        return impl->ToString();
    }
}

