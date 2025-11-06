// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;

namespace Alimer.Platforms.Apple;

[DebuggerDisplay("{DebuggerDisplay,nq}")]
internal readonly struct NSArray : IDisposable, IEquatable<NSArray>
{
    #region Selectors
    private static readonly Selector s_sel_count = "count";
    private static readonly Selector s_sel_objectAtIndex = "objectAtIndex:";
    #endregion

    public NSArray(nint handle) => Handle = handle;

    public NSArray()
    {
        var cls = new ObjectiveCClass("NSArray");
        Handle = cls.AllocInit();
    }

    public void Dispose()
    {
        ObjectiveC.objc_msgSend(Handle, Selectors.Release);
    }

    public nint Handle { get; }

    public ulong Count => ObjectiveC.ulong_objc_msgSend(Handle, s_sel_count);

    public static implicit operator NSArray(nint handle) => new(handle);
    public static implicit operator nint(NSArray value) => value.Handle;
    public unsafe T Object<T>(ulong index)
        where T : unmanaged
    {
        IntPtr value = ObjectiveC.IntPtr_objc_msgSend(Handle, s_sel_objectAtIndex, index);
        return Unsafe.AsRef<T>(&value);
    }

    public static bool operator ==(NSArray left, NSArray right) => left.Handle == right.Handle;
    public static bool operator !=(NSArray left, NSArray right) => left.Handle != right.Handle;
    public static bool operator ==(NSArray left, nint right) => left.Handle == right;
    public static bool operator !=(NSArray left, nint right) => left.Handle != right;
    public bool Equals(NSArray other) => Handle == other.Handle;
    /// <inheritdoc/>
    public override bool Equals([NotNullWhen(true)] object? obj) => obj is NSArray handle && Equals(handle);
    /// <inheritdoc/>
    public override readonly int GetHashCode() => Handle.GetHashCode();
    private readonly string DebuggerDisplay => $"{nameof(NSArray)} [0x{Handle:X}]";
}
