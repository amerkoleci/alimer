// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Text.Json.Serialization;

namespace Alimer.Engine;

partial class Entity
{
    public class SerializeOptions
    {
    }

    public JsonObject? Serialize(SerializeOptions options = default)
    {
        if (Flags.HasFlag(EntityFlags.NotSaved))
            return null;

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
            catch (System.Exception e)
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
                //var componentType = MetadataRegistry.GetType<Component>(componentTypeName, true);
                Component component = (Component)metadata.CreateObject()!;
                component.Deserialize(componentJson); 
                entity.Components.Add(component);
            }
        }

        return entity;
    }
}

internal static class ModuleInit
{
    [ModuleInitializer]
    public static void Register()
    {
        var componentMetaData = new ObjectTypeMetadata(typeof(Component));
        MetadataRegistry.Register(componentMetaData);
        MetadataRegistry.Register(new ObjectTypeMetadata(typeof(CameraComponent), () => new CameraComponent()));
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
