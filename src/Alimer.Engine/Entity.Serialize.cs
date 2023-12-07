// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.Serialization;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Alimer.Engine;

partial class Entity
{
    public class SerializeOptions
    {
    }

    public string? Serialize(SerializeOptions? options = default)
    {
        if (Flags.HasFlag(EntityFlags.NotSaved))
            return null;

        JsonSerializerOptions SerializerOptions = new()
        {
            WriteIndented = true,
            PropertyNameCaseInsensitive = true,
            IncludeFields = false
        };
        EntityJsonSerializerContext context = new (SerializerOptions);

        string jsonString = JsonSerializer.Serialize(this, context.Entity);
        return jsonString;
    }
}

[JsonSerializable(typeof(Entity))]
internal partial class EntityJsonSerializerContext : JsonSerializerContext
{
}
