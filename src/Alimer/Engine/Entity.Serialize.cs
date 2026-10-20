// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Rendering;
using Alimer.Serialization;

namespace Alimer.Engine;

partial class Entity
{
    internal const int Version = 1;

    public void Serialize(Serializer serializer)
    {
        if (Flags.HasFlag(EntityFlags.NotSaved))
            return;

        serializer.BeginObject();
        //using IDisposable _ = serializer.BeginArray();
        serializer.WriteVersion(Version);
        serializer.Write(Keys.Id, Id);
        serializer.Write(Keys.Name, Name);
        serializer.Write("Enum", LightType.Point);

        serializer.Write(Keys.Position, Transform.Position);
        //serializer.Write(Keys.Rotation, Transform.Rotation);
        //serializer.Write(Keys.Scale, Transform.Scale);
        //
        if (Components.Count > 0)
        {
            serializer.BeginArray(Keys.Components);
            //    using ObjectSerializer componentsArraySerializer = serializer.BeginArray(Keys.Components);
            foreach (Component component in Components)
            {
                if (component is TransformComponent)
                    continue;

                serializer.BeginObject();
                component.Serialize(serializer);
                serializer.EndObject();
            }

            serializer.EndArray();

            serializer.BeginArray("TestArray");
            serializer.Write(Vector3.One);
            serializer.Write(Vector3.UnitX);
            serializer.Write(Vector3.UnitY);
            serializer.Write(Vector3.UnitZ);
            serializer.EndArray();
        }

        serializer.EndObject();
    }

    public void Deserialize(Deserializer deserializer)
    {
        if (!deserializer.BeginObject())
            return;

        int version = deserializer.ReadVersion();
        Id = deserializer.ReadGuid(Keys.Id, Id);
        Name = deserializer.ReadString(Keys.Name, Name)!;
        LightType type = deserializer.ReadEnum("Enum", LightType.Point)!;
        Transform.Position = deserializer.ReadVector3(Keys.Position, Transform.Position);

        int componentCount = deserializer.BeginArray(Keys.Components);
        if (componentCount > 0)
        {
            for (int i = 0; i < componentCount; i++)
            {
                if (deserializer.BeginObject())
                {
                    string typeName = deserializer.ReadType();
                    //var metadata = MetadataRegistry.GetMetadata<Component, IObjectTypeMetadata>(typeName);
                    //IEnumerable<PropertyMetadata> props = MetadataRegistry.GetProperties(metadata.Type);

                    Component? component = MetadataRegistry.CreateInstance<Component>(typeName);
                    if (component is null)
                    {
                        deserializer.EndObject();
                        continue;
                    }

                    component.Deserialize(deserializer);
                    deserializer.EndObject();
                    Components.Add(component);
                }
            }
        }
        deserializer.EndArray();

        int testArrayElementCount = deserializer.BeginArray("TestArray");
        if(testArrayElementCount > 0)
        {
            Vector3[] vectors = new Vector3[testArrayElementCount];
            for (int i = 0; i < testArrayElementCount; i++)
            {
                vectors[i] = deserializer.ReadVector3();
            }
        }
        //serializer.Write(Vector3.One);
        //serializer.Write(Vector3.UnitX);
        //serializer.Write(Vector3.UnitY);
        //serializer.Write(Vector3.UnitZ);
        deserializer.EndArray();

        deserializer.EndObject();
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
