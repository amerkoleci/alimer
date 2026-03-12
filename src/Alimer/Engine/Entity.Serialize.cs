// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Rendering;
using Alimer.Serialization;

namespace Alimer.Engine;

partial class Entity
{
    internal const int Version = 1;

    public void Serialize(ObjectSerializer serializer)
    {
        if (Flags.HasFlag(EntityFlags.NotSaved))
            return;

        serializer.WriteVersion(Version);
        serializer.Write(Keys.Id, Id);
        serializer.Write(Keys.Name, Name);

        serializer.Write("Enum", LightType.Point);

        serializer.Write(Keys.Position, Transform.Position);
        serializer.Write(Keys.Rotation, Transform.Rotation);
        serializer.Write(Keys.Scale, Transform.Scale);

        if (Components.Count > 0)
        {
            using ObjectSerializer componentsArraySerializer = serializer.BeginArray(Keys.Components);
            foreach (Component component in Components)
            {
                if (component is TransformComponent)
                    continue;

                using ObjectSerializer componentSerializer = componentsArraySerializer.BeginObject();
                component.Serialize(componentSerializer);
            }
        }
    }

    public void Deserialize(ObjectDeserializer deserializer)
    {
        int version = deserializer.ReadVersion();
        Id = deserializer.ReadGuid(Keys.Id, Id);
        Name = deserializer.ReadString(Keys.Name, Name)!;
        LightType type = deserializer.ReadEnum<LightType>("Enum", LightType.Point)!;

#if false
        int componentCount = deserializer.BeginArray(Keys.Components);
        if (componentCount > 0)
        {
            for (int i = 0; i < componentCount; i++)
            {
                if (deserializer.BeginObject())
                {
                    int componentVersion = deserializer.ReadVersion();
                    string typeName = deserializer.ReadType();


                    var metadata = MetadataRegistry.GetMetadata<Component, IObjectTypeMetadata>();

                    Component component = MetadataRegistry.CreateInstance<Component>(typeName);

                    //Component? component = default;
                    //switch (typeName)
                    //{
                    //    case nameof(CameraComponent):
                    //        component = new CameraComponent();
                    //        break;
                    //}

                    if (component != null)
                    {
                        component.Deserialize(deserializer);
                    }
                    deserializer.EndObject();
                }
            }

            deserializer.EndArray();
        } 
#endif
    }

    internal static class Keys
    {
        public const string Id = "Id";
        public const string Name = "Name";
        public const string Position = "Position";
        public const string Rotation = "Rotation";
        public const string Scale = "Scale";

        public const string Components = "Components";
    }
}
