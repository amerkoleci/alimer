// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Text.Json.Nodes;
using Alimer.Serialization;

namespace Alimer.Engine;

partial class Entity
{
    internal const int Version = 1;

    public void Serialize(Serializer serializer)
    {
        if (Flags.HasFlag(EntityFlags.NotSaved))
            return;

        serializer.WriteVersion(Version);
        serializer.Write(Keys.Id, Id);
        serializer.Write(Keys.Name, Name);

        //json.Add("Position", JsonValue.Create(Transform.Position));

        if (Components.Count > 0)
        {
            serializer.BeginArray(Keys.Components);
            foreach (Component component in Components)
            {
                if (component is TransformComponent)
                    continue;

                try
                {
                    string componentTypeName = component.GetType().Name;
                    serializer.BeginObject(default, componentTypeName, component.ComponentVersion);
                    component.Serialize(serializer);
                    serializer.EndObject();
                }
                catch (Exception)
                {
                }
            }
            serializer.EndArray();
        }
    }

    public void Deserialize(Deserializer deserializer)
    {
        int version = deserializer.ReadVersion();
        Id = deserializer.ReadGuid(Keys.Id, Id);
        Name = deserializer.ReadString(Keys.Name, Name)!;

        int componentCount = deserializer.BeginArray(Keys.Components);
        if (componentCount > 0)
        {
            for (int i = 0; i < componentCount; i++)
            {
                if (deserializer.BeginObject())
                {
                    int componentVersion = deserializer.ReadVersion();
                    string typeName = deserializer.ReadType();

                    Component? component = default;
                    switch (typeName)
                    {
                        case nameof(CameraComponent):
                            component = new CameraComponent();
                            break;
                    }

                    if (component != null)
                    {
                        component.Deserialize(deserializer);
                    }
                    deserializer.EndObject();
                }
            }

            deserializer.EndArray();
        }

    }

    internal static class Keys
    {
        public const string Id = "Id";
        public const string Name = "Name";

        public const string Components = "Components";
    }

#if TODO
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
#endif
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
