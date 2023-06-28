// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

[AttributeUsage(AttributeTargets.Method)]
public sealed class MonoPInvokeCallbackAttribute : Attribute
{
    public MonoPInvokeCallbackAttribute(Type type)
    {
        Type = type;
    }

    public Type Type { get; }
}
