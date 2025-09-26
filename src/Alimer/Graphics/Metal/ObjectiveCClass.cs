// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;

namespace Alimer.Graphics.Metal;

[DebuggerDisplay("{DebuggerDisplay,nq}")]
internal readonly partial struct ObjectiveCClass : IEquatable<ObjectiveCClass>
{
    public ObjectiveCClass(nint handle)
    {
        Handle = handle;
    }

    public ObjectiveCClass(string name)
    {
        Handle = ObjectiveC.objc_getClass(name);

        if (Handle == 0)
        {
            throw new InvalidOperationException($"Failed to get class {name}!");
        }
    }
    public nint Handle { get; }
    public readonly bool IsNull => Handle == 0;
    public readonly bool IsNotNull => Handle != 0;

#if DEBUG
    public string Name => ObjectiveC.class_getName(this);

    public nint GetProperty(string propertyName)
    {
        return ObjectiveC.class_getProperty(this, propertyName);
    }
#endif

    public ObjectiveCClass AllocInit()
    {
        nint value = ObjectiveC.IntPtr_objc_msgSend(Handle, Selectors.Alloc);
        ObjectiveC.objc_msgSend(value, Selectors.Init);
        return new(value);
    }

    public ObjectiveCClass Alloc()
    {
        nint value = ObjectiveC.IntPtr_objc_msgSend(Handle, Selectors.Alloc);
        return new(value);
    }

    public unsafe T Alloc<T>()
        where T : unmanaged
    {
        nint value = ObjectiveC.IntPtr_objc_msgSend(Handle, Selectors.Alloc);
        return Unsafe.AsRef<T>(&value);
    }

    public unsafe T AllocInit<T>()
        where T : unmanaged
    {
        nint value = ObjectiveC.IntPtr_objc_msgSend(Handle, Selectors.Alloc);
        ObjectiveC.objc_msgSend(value, Selectors.Init);
        return Unsafe.AsRef<T>(&value);
    }

    public static ObjectiveCClass Null => new(0);
    public static implicit operator ObjectiveCClass(nint handle) => new(handle);
    public static implicit operator nint(ObjectiveCClass selector) => selector.Handle;
    public static implicit operator ObjectiveCClass(string value) => new(value);

    public static bool operator ==(ObjectiveCClass left, ObjectiveCClass right) => left.Handle == right.Handle;
    public static bool operator !=(ObjectiveCClass left, ObjectiveCClass right) => left.Handle != right.Handle;
    public static bool operator ==(ObjectiveCClass left, nint right) => left.Handle == right;
    public static bool operator !=(ObjectiveCClass left, nint right) => left.Handle != right;
    public bool Equals(ObjectiveCClass other) => Handle == other.Handle;
    /// <inheritdoc/>
    public override bool Equals([NotNullWhen(true)] object? obj) => obj is ObjectiveCClass handle && Equals(handle);
    /// <inheritdoc/>
    public override readonly int GetHashCode() => Handle.GetHashCode();
    private readonly string DebuggerDisplay => $"{nameof(ObjectiveCClass)} [0x{Handle:X}]";
}
