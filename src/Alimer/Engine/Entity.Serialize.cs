// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Text.Json.Serialization;
using System.Text.Json.Serialization.Metadata;
using TerraFX.Interop.WinRT;

namespace Alimer.Engine;

partial class Entity
{
    public class SerializeOptions
    {
    }

    public JsonObject? Serialize(SerializeOptions? options = default)
    {
        if (Flags.HasFlag(EntityFlags.NotSaved))
            return null;

        var options2 = new JsonSerializerOptions
        {
            WriteIndented = true,
            TypeInfoResolver = new SerializableTypeResolver(),
            //Converters = { new SerializableTypeConverter() }
        };

        string jsonOps = JsonSerializer.Serialize(this, options2);
        //string json2 = JsonSerializer.Serialize(this, EntityJsonContext.Default.Entity);
        //var entityLoaded = JsonSerializer.Deserialize(json2, EntityJsonContext.Default.Entity);

        JsonObject json = new()
        {
            { "Id", Id },
            { "Name", Name },
        };

        //json.Add("Position", JsonValue.Create(Transform.Position));

        var components = new JsonArray();

        foreach (Component component in Components)
        {
            if (component is null) continue;

            if (component is TransformComponent)
                continue;

            try
            {
                var result = component.Serialize(options);
                if (result is null) continue;

                components.Add(result);
            }
            catch (Exception)
            {
            }
        }

        json.Add("Components", components);
        return json;
    }

    public static Entity? Deserialize([StringSyntax(StringSyntaxAttribute.Json)] string json)
    {
        JsonObject node = (JsonObject)JsonNode.Parse(json)!;
        return DeserializeElement(node);
    }

    public static Entity? DeserializeElement(JsonObject node)
    {
        var serializedVersion = (int)(node["Version"] ?? 0);
        string name = node.GetPropertyValue("Name", string.Empty);

        Entity entity = new Entity(name);

        if (node["Components"] is JsonArray componentArray)
        {
            for (int componentIndex = 0; componentIndex < componentArray.Count; componentIndex++)
            {
                JsonNode? jsonNodeComponent = componentArray[componentIndex];

                if (jsonNodeComponent is not JsonObject componentJson)
                {
                    continue;
                }

                string componentTypeName = componentJson.GetPropertyValue("Type", "");

                ITypeMetadata metadata = MetadataRegistry.GetMetadata(componentTypeName);
                if (metadata.CreateObject is null)
                {
                    throw new InvalidOperationException("Metadata for type " + componentTypeName + " does not have a CreateObject function.");
                }

                //var componentType = MetadataRegistry.GetType<Component>(componentTypeName, true);
                Component component = (Component)metadata.CreateObject();
                component.Deserialize(componentJson);
                entity.Components.Add(component);
            }
        }

        return entity;
    }

    public class SerializableTypeResolver : DefaultJsonTypeInfoResolver
    {
        public override JsonTypeInfo GetTypeInfo(Type type, JsonSerializerOptions options)
        {
            JsonTypeInfo jsonTypeInfo = base.GetTypeInfo(type, options);

            Type basePointType = typeof(Component);
            if (jsonTypeInfo.Type == basePointType)
            {
                MetadataRegistry.Prepare();

                jsonTypeInfo.PolymorphismOptions = new JsonPolymorphismOptions
                {
                    TypeDiscriminatorPropertyName = "__type",
                    IgnoreUnrecognizedTypeDiscriminators = true,
                    UnknownDerivedTypeHandling = JsonUnknownDerivedTypeHandling.FailSerialization,
                    //DerivedTypes =
                    //{
                    //    new JsonDerivedType(typeof(TransformComponent), nameof(TransformComponent)),
                    //    new JsonDerivedType(typeof(CameraComponent), nameof(CameraComponent)),
                    //}
                };

                //List<JsonDerivedType> derivedTypes = [];
                IReadOnlySet<Type> subtypes = MetadataRegistry.GetSubtypes(typeof(Component));
                foreach (Type subType in subtypes)
                {
                    jsonTypeInfo.PolymorphismOptions.DerivedTypes.Add(new JsonDerivedType(subType, subType.Name));
                }
            }

            return jsonTypeInfo;
        }
    }
}

internal static class ModuleInit
{
#pragma warning disable CA2255
    [ModuleInitializer]
#pragma warning restore CA2255
    public static void Register()
    {
        var componentMetaData = new ObjectTypeMetadata<Component>();
        MetadataRegistry.Register(componentMetaData);
        MetadataRegistry.Register(new ObjectTypeMetadata<TransformComponent>(() => new TransformComponent()));
        MetadataRegistry.Register(new ObjectTypeMetadata<CameraComponent>(() => new CameraComponent()));

    }
}

public static class JsonUtils
{
    public static T GetPropertyValue<T>(this JsonObject? node, string name, in T defaultValue)
    {
        if (node is null)
            return defaultValue;

        if (!node.TryGetPropertyValue(name, out JsonNode? value))
            return defaultValue;

        return value!.GetValue<T>();
    }
}
