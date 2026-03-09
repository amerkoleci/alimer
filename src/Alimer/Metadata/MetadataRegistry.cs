// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.Concurrent;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Reflection;

namespace Alimer;

public enum MetadataTypeKind
{
    /// <summary>
    /// A primitive type (e.g `int`, `short`, `double`...)
    /// </summary>
    Primitive,
    ValueType,
    Enum,
    Object,
}

/// <summary>
/// Gets the visibility of an element.
/// </summary>
public enum MetadataTypeVisibility
{
    /// <summary>
    /// `public` visibility
    /// </summary>
    Public,

    /// <summary>
    /// `protected` visibility
    /// </summary>
    Protected,

    /// <summary>
    /// `private` visibility
    /// </summary>
    Private,
}

public interface ITypeMetadata
{
    Type Type { get; }
    MetadataTypeKind Kind { get; }
    Func<object>? CreateObject { get; set; }
    MetadataTypeVisibility Visibility { get; }
    int SizeOf { get; }
}

public interface IValueTypeMetadata : ITypeMetadata
{
    MetadataTypeKind ITypeMetadata.Kind => MetadataTypeKind.ValueType;
}

public interface IPrimitiveTypeMetadata : IValueTypeMetadata
{
    MetadataTypeKind ITypeMetadata.Kind => MetadataTypeKind.Primitive;
    MetadataTypeVisibility ITypeMetadata.Visibility => MetadataTypeVisibility.Public;
}

public interface IEnumTypeMetadata : ITypeMetadata
{
    MetadataTypeKind ITypeMetadata.Kind => MetadataTypeKind.Enum;
    IPrimitiveTypeMetadata UnderlyingType { get; }
    bool IsFlags { get; }
    IReadOnlyList<EnumItem> Items { get; }
}

public interface IObjectTypeMetadata : ITypeMetadata
{
    MetadataTypeKind ITypeMetadata.Kind => MetadataTypeKind.Object;

    IReadOnlyList<PropertyMetadata> Properties { get; }
}

public class EnumItem
{
    /// <summary>
    /// Creates a new instance of this enum item.
    /// </summary>
    /// <param name="name">Name of the enum item.</param>
    /// <param name="value">Associated value of this enum item.</param>
    public EnumItem(string name, long value)
    {
        Name = name ?? throw new ArgumentNullException(nameof(name));
        Value = value;
    }

    /// <inheritdoc />
    public string Name { get; set; }

    /// <summary>
    /// Gets the value of this enum item.
    /// </summary>
    public long Value { get; set; }
}

public class PrimitiveTypeMetadata : IPrimitiveTypeMetadata
{
    public PrimitiveTypeMetadata(Type type, int sizeOf)
    {
        ArgumentNullException.ThrowIfNull(type, nameof(type));
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(sizeOf, nameof(sizeOf));

        Type = type;
        SizeOf = sizeOf;
    }

    public Type Type { get; }

    public Func<object>? CreateObject { get; set; }
    public int SizeOf { get; }
}

public class EnumTypeMetadata<T, TUnderlying> : IEnumTypeMetadata
    where T : struct, Enum
    where TUnderlying : unmanaged
{
    public EnumTypeMetadata(MetadataTypeVisibility visibility = MetadataTypeVisibility.Public)
    //: base(typeof(T), MetadataRegistry.GetPrimitiveTypeMetadata<int>(), visibility)
    {
        Type = typeof(T);
        UnderlyingType = MetadataRegistry.GetPrimitiveTypeMetadata<TUnderlying>();
        CreateObject = () => new T();
        SizeOf = UnderlyingType.SizeOf;
        Visibility = visibility;
    }

    public Type Type { get; }
    public IPrimitiveTypeMetadata UnderlyingType { get; }
    public Func<object>? CreateObject { get; set; }
    public int SizeOf { get; }
    public MetadataTypeVisibility Visibility { get; set; }
    public bool IsFlags { get; }
    public required IReadOnlyList<EnumItem> Items { get; init; }
}

public sealed class PropertyMetadata
{
    public required string Name { get; init; }

    /// <summary>
    /// The declaring type of the property or field.
    /// </summary>
    public required Type DeclaringType { get; init; } = null!;

    public Type PropertyType { get; init; } = default!;
}

public class ObjectTypeMetadata : IObjectTypeMetadata
{
    public Type Type { get; }

    public IReadOnlyList<PropertyMetadata> Properties => throw new NotImplementedException();

    public ObjectTypeMetadata(Type type, Func<object>? createObject = default)
    {
        ArgumentNullException.ThrowIfNull(type, nameof(type));

        Type = type;
        var baseAttributes = type.GetCustomAttributes<System.Attribute>(true);

        foreach (var prop in type.GetProperties(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.DeclaredOnly))
        {
            var attributes = prop.GetCustomAttributes<System.Attribute>(true);
            if (attributes.OfType<MetaAttribute>().Any() || baseAttributes.OfType<MetaAttribute>().Any())
            {
                // This is a property we want to include in the metadata.
            }
        }

        //var test = MetadataRegistry.GetMetadata(type.BaseType.n);
        CreateObject = createObject;
        SizeOf = 0;
    }

    public Func<object>? CreateObject { get; set; }
    public int SizeOf { get; }
    public MetadataTypeVisibility Visibility { get; set; }
}

public static class MetadataRegistry
{
    private static readonly ConcurrentDictionary<Type, ITypeMetadata> s_types = new();

    private static readonly ConcurrentDictionary<Type, IEnumerable<PropertyMetadata>> s_properties = new();

    static MetadataRegistry()
    {
        // Register built-in primitive types
        RegisterPrimitiveType<bool>();
        RegisterPrimitiveType<byte>();
        RegisterPrimitiveType<sbyte>();
        RegisterPrimitiveType<short>();
        RegisterPrimitiveType<ushort>();
        RegisterPrimitiveType<int>();
        RegisterPrimitiveType<uint>();
        RegisterPrimitiveType<long>();
        RegisterPrimitiveType<ulong>();
        RegisterPrimitiveType<float>();
        RegisterPrimitiveType<double>();
        RegisterPrimitiveType<decimal>();
        RegisterPrimitiveType<char>();
    }

    public static unsafe void RegisterPrimitiveType<T>()
        where T : unmanaged
    {
        Type type = typeof(T);
        if (s_types.ContainsKey(type))
            return;

        int size = sizeof(T);
        PrimitiveTypeMetadata metadata = new(type, size)
        {
            CreateObject = () => new T(),
        };
        s_types[type] = metadata;
    }

    public static void Register(ITypeMetadata metadata)
    {
        ArgumentNullException.ThrowIfNull(metadata);

        if (s_types.ContainsKey(metadata.Type))
            return;

        s_types[metadata.Type] = metadata;
    }

    public static IEnumerable<PropertyMetadata> GetProperties(Type type)
    {
        if (!s_types.TryGetValue(type, out ITypeMetadata? metadata))
        {
            return [];
        }

        if (metadata is not IObjectTypeMetadata objectTypeMetadata)
        {
            return [];
        }


        if (!s_properties.TryGetValue(type, out IEnumerable<PropertyMetadata>? properties))
        {
            // Cache the properties for a type so we never have to do this again.
            s_properties[type] =
              GetTypeAndBaseTypes(type)
                .Select(GetMetadata)
                .OfType<IObjectTypeMetadata>()
                .SelectMany(metadata => metadata.Properties)
                .Distinct()
                .OrderBy(property => property.Name);
        }

        return s_properties[type];
    }

    private static IEnumerable<Type> GetTypeAndBaseTypes(Type type)
    {
        Type? currentType = type;

        do
        {
            if (s_types.TryGetValue(currentType, out ITypeMetadata? value) && value is IObjectTypeMetadata)
            {
                yield return currentType;
            }

            currentType = currentType.BaseType;
        } while (currentType != null);
    }

    public static bool HasMetadata(Type type) => s_types.ContainsKey(type);

    public static bool HasMetadata<T>() => s_types.ContainsKey(typeof(T));

    public static ITypeMetadata GetMetadata(Type type)
    {
        if (s_types.TryGetValue(type, out ITypeMetadata? metadata))
        {
            return metadata;
        }

        throw new InvalidOperationException($"No metadata found for type {type.FullName}");
    }
    public static ITypeMetadata GetMetadata(string typeName)
    {
        foreach (var kvp in s_types)
        {
            if (kvp.Key.Name == typeName)
            {
                return kvp.Value;
            }
        }

        throw new InvalidOperationException($"No metadata found for type name {typeName}");
    }


    public static TTypeMetadata GetMetadata<T, TTypeMetadata>()
        where TTypeMetadata : ITypeMetadata
    {
        ITypeMetadata metadata = GetMetadata(typeof(T));
        if (metadata is not TTypeMetadata typedMetadata)
        {
            throw new InvalidOperationException($"Metadata for type {typeof(T).FullName} is not of type {typeof(TTypeMetadata).FullName}.");
        }

        return typedMetadata;
    }

    public static IPrimitiveTypeMetadata GetPrimitiveTypeMetadata<T>()
        where T : unmanaged => GetMetadata<T, IPrimitiveTypeMetadata>();

    public static bool TryGetMetadata(Type type, [NotNullWhen(true)] out ITypeMetadata? metadata)
    {
        return s_types.TryGetValue(type, out metadata);
    }

    public static bool TryGetMetadata<T>([NotNullWhen(true)] out ITypeMetadata? metadata)
    {
        return TryGetMetadata(typeof(T), out metadata);
    }

    public static bool TryGetMetadata<T, TTypeMetadata>([NotNullWhen(true)] out TTypeMetadata? metadata)
        where TTypeMetadata : ITypeMetadata
    {
        if (TryGetMetadata<T>(out ITypeMetadata? untypedMetadata)
            && untypedMetadata is TTypeMetadata typedMetadata)
        {
            metadata = typedMetadata;
            return true;
        }

        metadata = default;
        return false;
    }

    public static IEnumerable<ITypeMetadata> GetAllTypes()
    {
        return s_types.Values;
    }
}
