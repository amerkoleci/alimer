// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Text.Json.Serialization;

namespace Alimer.Engine;

partial class Entity
{
    public class SerializeOptions
    {
    }

    //public virtual JsonObject? Serialize(SerializeOptions? options = null)
    //{
    //    if (Flags.HasFlag(EntityFlags.NotSaved))
    //        return null;

    //    JsonObject json = new()
    //    {
    //        { "Id", Id },
    //        { "Name", Name },
    //    };

    //    if (Transform.Position != Vector3.Zero) json.Add("Position", JsonValue.Create(Transform.Position));
    //    return json;
    //}

    //public string? Serialize(SerializeOptions? options = default)
    //{
    //    if (Flags.HasFlag(EntityFlags.NotSaved))
    //        return null;

    //    JsonSerializerOptions SerializerOptions = new()
    //    {
    //        WriteIndented = true,
    //        PropertyNameCaseInsensitive = true,
    //        IncludeFields = false
    //    };
    //    EntityJsonSerializerContext context = new(SerializerOptions);

    //    string jsonString = JsonSerializer.Serialize(this, context.Entity);
    //    return jsonString;
    //}
}

[JsonSerializable(typeof(Entity))]
internal partial class EntityJsonSerializerContext : JsonSerializerContext
{
}
