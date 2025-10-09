// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Engine;

[AttributeUsage(AttributeTargets.Class, AllowMultiple = true)]
public sealed class DefaultEntitySystemAttribute(Type type) : Attribute
{
    public Type Type { get; } = type;
}
