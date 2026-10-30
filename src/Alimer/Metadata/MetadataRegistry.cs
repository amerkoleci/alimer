// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.Concurrent;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Runtime.CompilerServices;

namespace Alimer;

public interface ITypeMetadata
{
    Type Type { get; }
    Func<object>? CreateObject { get; }
    int SizeOf { get; }
    public bool IsEnum => Type.IsEnum;
    public bool IsStruct => Type.IsValueType && !Type.IsEnum;
    public bool IsAbstract => Type.IsAbstract;
}

public interface IObjectTypeMetadata : ITypeMetadata
{
    IReadOnlyList<PropertyMetadata> Properties { get; }
}

//public interface ITypeMetadataProvider
//{
//    public static abstract ITypeMetadata TypeMetadata { get; }
//}

#if TODO
public interface IEnumTypeMetadata : ITypeMetadata
{
    Type UnderlyingType { get; }
    bool IsFlags { get; }
    IReadOnlyList<EnumItem> Items { get; }
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

public class EnumTypeMetadata<TEnum, TUnderlying> : IEnumTypeMetadata
    where TEnum : struct, Enum
    where TUnderlying : unmanaged
{
    public EnumTypeMetadata()
    {
        Type = typeof(TEnum);
        UnderlyingType = typeof(TUnderlying);
        CreateObject = () => new TEnum();
        SizeOf = Unsafe.SizeOf<TUnderlying>();
        IsFlags = typeof(TEnum).GetCustomAttribute<FlagsAttribute>() != null;
    }

    public Type Type { get; }
    public Type UnderlyingType { get; }
    public Func<object>? CreateObject { get; set; }
    public int SizeOf { get; }
    public bool IsFlags { get; }
    public required IReadOnlyList<EnumItem> Items { get; init; }
} 
#endif

public abstract class MemberMetadata
{
    protected MemberMetadata(string name, Type memberType, Attribute[]? attributes = default)
    {
        Name = name ?? throw new ArgumentNullException(nameof(name));
        MemberType = memberType ?? throw new ArgumentNullException(nameof(memberType));
        Attributes = attributes ?? [];

        foreach (Attribute attribute in Attributes)
        {
            if (attribute is System.ComponentModel.DataAnnotations.DisplayAttribute display)
            {
                if (!string.IsNullOrWhiteSpace(display.Name)) Name = display.Name;
                if (!string.IsNullOrWhiteSpace(display.GroupName)) GroupName = display.GroupName;
                if (!string.IsNullOrWhiteSpace(display.Description)) Description = display.Description;
                if (!string.IsNullOrWhiteSpace(display.ShortName)) Icon ??= display.ShortName;

                Order = display.GetOrder() ?? Order;
            }

            if (attribute is System.ComponentModel.CategoryAttribute categoryAttribute)
            {
                GroupName = categoryAttribute.Category;
            }

            if (attribute is System.ComponentModel.DisplayNameAttribute displayNameAttribute)
            {
                DisplayName = displayNameAttribute.DisplayName;
            }

            if (attribute is System.ComponentModel.DescriptionAttribute descriptionAttribute)
            {
                Description = descriptionAttribute.Description;
            }

            if (attribute is System.Text.Json.Serialization.JsonPropertyOrderAttribute jsonPropertyOrderAttribute)
            {
                Order = jsonPropertyOrderAttribute.Order;
            }
        }
    }

    protected MemberMetadata(MemberInfo memberInfo)
    {
        ArgumentNullException.ThrowIfNull(memberInfo, nameof(memberInfo));

        Name = memberInfo.Name;
        Attributes = memberInfo.GetCustomAttributes().ToArray();

        if (memberInfo is PropertyInfo propertyInfo)
        {
            MemberType = propertyInfo.PropertyType;
        }
        else if (memberInfo is FieldInfo fieldInfo)
        {
            MemberType = fieldInfo.FieldType;
        }
        else
        {
            throw new ArgumentException("MemberInfo must be of type PropertyInfo or FieldInfo.", nameof(memberInfo));
        }

        foreach (Attribute attribute in Attributes)
        {
            if (attribute is System.ComponentModel.DataAnnotations.DisplayAttribute display)
            {
                if (!string.IsNullOrWhiteSpace(display.Name)) Name = display.Name;
                if (!string.IsNullOrWhiteSpace(display.GroupName)) GroupName = display.GroupName;
                if (!string.IsNullOrWhiteSpace(display.Description)) Description = display.Description;
                if (!string.IsNullOrWhiteSpace(display.ShortName)) Icon ??= display.ShortName;

                Order = display.GetOrder() ?? Order;
            }

            if (attribute is System.ComponentModel.CategoryAttribute categoryAttribute)
            {
                GroupName = categoryAttribute.Category;
            }

            if (attribute is System.ComponentModel.DisplayNameAttribute displayNameAttribute)
            {
                DisplayName = displayNameAttribute.DisplayName;
            }

            if (attribute is System.ComponentModel.DescriptionAttribute descriptionAttribute)
            {
                Description = descriptionAttribute.Description;
            }

            if (attribute is System.Text.Json.Serialization.JsonPropertyOrderAttribute jsonPropertyOrderAttribute)
            {
                Order = jsonPropertyOrderAttribute.Order;
            }
        }
    }

    public string Name { get; }
    public Type MemberType { get; }
    public IReadOnlyList<Attribute> Attributes { get; }

    /// <summary>
    /// Group or category name of this type or member.
    /// </summary>
    public string? GroupName { get; set; }
    public string? DisplayName { get; set; }
    public string? Description { get; set; }
    public string? Icon { get; set; }
    public int? Order { get; set; }
}

public sealed class PropertyMetadata : MemberMetadata
{
    public PropertyMetadata(string name, Type memberType, Attribute[]? attributes = default)
        : base(name, memberType, attributes)
    {
    }

    public PropertyMetadata(MemberInfo memberInfo)
        : base(memberInfo)
    {
        if (memberInfo is PropertyInfo propertyInfo)
        {
            IsField = false;

            Getter = propertyInfo.CanRead ? (object instance) => propertyInfo.GetValue(instance) : null;
            Setter = propertyInfo.CanWrite ? (object instance, object? value) => propertyInfo.SetValue(instance, value) : null;
        }
        else if (memberInfo is FieldInfo fieldInfo)
        {
            IsField = true;

            Getter = instance => fieldInfo.GetValue(instance);
            Setter = (instance, value) => fieldInfo.SetValue(instance, value);
        }
        else
        {
            throw new ArgumentException("MemberInfo must be of type PropertyInfo or FieldInfo.", nameof(memberInfo));
        }

    }

    public bool IsField { get; }
    public bool HasDefaultValue { get; set; }
    public Func<object, object?>? Getter { get; set; }
    public Action<object, object?>? Setter { get; set; }
    //public bool IsSerializable => Getter is not null && Setter is not null;
}

public class ObjectTypeMetadata : IObjectTypeMetadata
{
    public Type Type { get; }

    public Func<object>? CreateObject { get; }
    public IReadOnlyList<PropertyMetadata> Properties { get; }
    public IReadOnlyList<Attribute> Attributes { get; }

    public ObjectTypeMetadata(Type type, Func<object>? createObject = default, PropertyMetadata[]? properties = default, Attribute[]? attributes = default)
    {
        Type = type ?? throw new ArgumentNullException(nameof(type));
        CreateObject = createObject;
        Properties = properties ?? [];
        Attributes = attributes ?? [];

        foreach (Attribute attribute in Attributes)
        {
            if (attribute is System.ComponentModel.DataAnnotations.DisplayAttribute display)
            {
            //    if (!string.IsNullOrWhiteSpace(display.Name)) Name = display.Name;
                if (!string.IsNullOrWhiteSpace(display.GroupName)) GroupName = display.GroupName;
            //    if (!string.IsNullOrWhiteSpace(display.Description)) Description = display.Description;
            //    if (!string.IsNullOrWhiteSpace(display.ShortName)) Icon ??= display.ShortName;

            //    Order = display.GetOrder() ?? Order;
            }

            if (attribute is System.ComponentModel.CategoryAttribute categoryAttribute)
            {
                GroupName = categoryAttribute.Category;
            }

            if (attribute is System.ComponentModel.DisplayNameAttribute displayNameAttribute)
            {
                DisplayName = displayNameAttribute.DisplayName;
            }

            //if (attribute is System.ComponentModel.DescriptionAttribute descriptionAttribute)
            //{
            //    Description = descriptionAttribute.Description;
            //}

            //if (attribute is System.Text.Json.Serialization.JsonPropertyOrderAttribute jsonPropertyOrderAttribute)
            //{
            //    Order = jsonPropertyOrderAttribute.Order;
            //}
        }
    }

    public int SizeOf { get; }
    /// <summary>
    /// Group or category name of this type or member.
    /// </summary>
    public string? GroupName { get; set; }

    public string? DisplayName { get; set; }
}

public static class MetadataRegistry
{
    private static readonly ConcurrentDictionary<Type, ITypeMetadata> s_types = new();
    private static readonly ConcurrentDictionary<Type, HashSet<Type>> s_typesByBaseType = new();
    private static readonly ConcurrentDictionary<Type, HashSet<Type>> s_typesByAncestor = new();

    private static readonly ConcurrentDictionary<Type, IEnumerable<PropertyMetadata>> s_properties = new();

#if TODO
    public static IEnumTypeMetadata RegisterEnumType<TEnum, TUnderlying>()
    where TEnum : struct,
    Enum where TUnderlying : unmanaged
    {
        Type type = typeof(TEnum);
        if (s_types.TryGetValue(type, out ITypeMetadata? existingMetadata))
        {
            if (existingMetadata is not IEnumTypeMetadata enumTypeMetadata)
            {
                throw new InvalidOperationException($"Metadata for type {type.Name} is not of type {typeof(IEnumTypeMetadata).Name}.");
            }

            return enumTypeMetadata;
        }

        EnumTypeMetadata<TEnum, TUnderlying> metadata = new()
        {
            Items = Enum.GetValues<TEnum>()
                    .Select(value => new EnumItem(Enum.GetName(value)!, Convert.ToInt64(value)))
                    .ToList()
        };
        s_types[type] = metadata;
        return metadata;
    } 
#endif

    public static IObjectTypeMetadata RegisterObjectType(
        Type type,
        Func<object>? createObject = default,
        PropertyMetadata[]? properties = default,
        Attribute[]? attributes = default)
    {
        if (s_types.TryGetValue(type, out ITypeMetadata? existingMetadata))
        {
            if (existingMetadata is not IObjectTypeMetadata objectTypeMetadata)
            {
                throw new InvalidOperationException($"Metadata for type {type.Name} is not of type {typeof(IObjectTypeMetadata).Name}.");
            }

            return objectTypeMetadata;
        }

        ObjectTypeMetadata metadata = new(type, createObject, properties, attributes);
        s_types[type] = metadata;
        return metadata;
    }

    public static void Prepare()
    {
        ComputeTypesByBaseType(s_types.Keys);
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


        return s_properties.GetOrAdd(type, static key =>
            GetTypeAndBaseTypes(key)
                .Select(GetMetadata)
                .OfType<IObjectTypeMetadata>()
                .SelectMany(metadata => metadata.Properties)
                .Distinct()
                .OrderBy(property => property.Name)
                .ToArray());
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

    public static TTypeMetadata GetMetadata<TTypeMetadata>(Type type)
        where TTypeMetadata : ITypeMetadata
    {
        ITypeMetadata metadata = GetMetadata(type);
        if (metadata is not TTypeMetadata typedMetadata)
        {
            throw new InvalidOperationException($"Metadata for type {type.FullName} is not of type {typeof(TTypeMetadata).FullName}.");
        }
        return typedMetadata;
    }

    public static ITypeMetadata GetMetadata(string typeName)
    {
        foreach (KeyValuePair<Type, ITypeMetadata> kvp in s_types)
        {
            if (kvp.Key.Name == typeName)
            {
                return kvp.Value;
            }
        }

        throw new InvalidOperationException($"No metadata found for type name {typeName}");
    }

    public static T CreateInstance<T>()
    {
        ITypeMetadata metadata = GetMetadata(typeof(T));
        if (metadata.CreateObject == null)
        {
            throw new InvalidOperationException($"Type {typeof(T).FullName} does not have a parameterless constructor or factory method.");
        }

        return (T)metadata.CreateObject();
    }

    public static T? CreateInstance<T>(string typeName)
    {
        ITypeMetadata metadata = GetMetadata(typeName);
        if (metadata.CreateObject == null)
        {
            throw new InvalidOperationException($"Type {typeName} does not have a parameterless constructor or factory method.");
        }

        return (T?)metadata.CreateObject();
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

    public static TTypeMetadata GetMetadata<T, TTypeMetadata>(string typeName)
        where TTypeMetadata : ITypeMetadata
    {
        ITypeMetadata metadata = GetMetadata(typeName);
        if (metadata is not TTypeMetadata typedMetadata)
        {
            throw new InvalidOperationException($"Metadata for type {typeName} is not of type {typeof(TTypeMetadata).FullName}.");
        }

        return typedMetadata;
    }

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

    private static readonly IReadOnlySet<Type> s_emptyTypeSet = new HashSet<Type>();

    public static IReadOnlySet<Type> GetSubtypes(Type type) =>
        s_typesByBaseType.TryGetValue(type, out HashSet<Type>? subtypes)
        ? subtypes
        : s_emptyTypeSet;

    public static IReadOnlySet<Type> GetDescendantSubtypes(Type type)
    {
        CacheDescendants(type);
        return s_typesByAncestor[type];
    }

    private static void CacheDescendants(Type type)
    {
        if (s_typesByAncestor.ContainsKey(type))
        {
            return;
        }

        s_typesByAncestor[type] = FindDescendants(type);
    }

    private static void ComputeTypesByBaseType(IEnumerable<Type> types)
    {
        // Iterate through each type in the registry and its base types,
        // constructing a flat map of base types to their immediately derived types.
        // The beauty of this approach is that it discovers base types which may be
        // in other modules, and works in reflection-free mode since BaseType is
        // always supported by every C# environment, even AOT environments.
        foreach (Type type in types)
        {
            Type lastType = type;
            Type? baseType = type.BaseType;

            // As far as we know, any type could be a base type.
            if (!s_typesByBaseType.ContainsKey(type))
            {
                s_typesByBaseType[type] = [];
            }

            while (baseType != null)
            {
                if (!s_typesByBaseType.TryGetValue(baseType, out HashSet<Type>? existingSet))
                {
                    existingSet = [];
                    s_typesByBaseType[baseType] = existingSet;
                }
                existingSet.Add(lastType);

                lastType = baseType;
                baseType = lastType.BaseType;
            }
        }
    }


    private static HashSet<Type> FindDescendants(Type type)
    {
        HashSet<Type> descendants = [];
        Queue<Type> queue = new();
        queue.Enqueue(type);

        while (queue.Count > 0)
        {
            Type currentType = queue.Dequeue();
            descendants.Add(currentType);

            if (s_typesByBaseType.TryGetValue(currentType, out HashSet<Type>? children))
            {
                foreach (Type child in children)
                {
                    queue.Enqueue(child);
                }
            }
        }

        descendants.Remove(type);

        return descendants;
    }

    public static IEnumerable<ITypeMetadata> GetAllTypes()
    {
        return s_types.Values;
    }
}
