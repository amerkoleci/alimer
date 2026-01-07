// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using static Alimer.Platforms.Apple.ObjectiveC;

namespace Alimer.Platforms.Apple;

internal readonly partial struct NSString : IDisposable
{
    #region Selectors
    private static readonly ObjectiveCClass s_class = new(nameof(NSString));
    private static Selector sel_utf8String => "UTF8String";
    private static Selector sel_initWithCharacters => "initWithCharacters:length:";
    #endregion 

    public nint Handle { get; }

    public NSString(nint handle) => Handle = handle;

    public NSString()
    {
        Handle = s_class.AllocInit();
    }

    public void Dispose()
    {
        objc_msgSend(Handle, Selectors.Release);
    }

    public static unsafe implicit operator NSString(string? value)
    {
        if (string.IsNullOrEmpty(value))
        {
            return new NSString(0);
        }

        NSString nss = s_class.Alloc<NSString>();

        fixed (char* utf16Ptr = value)
        {
            nuint length = (nuint)value.Length;
            IntPtr newString = IntPtr_objc_msgSend(nss.Handle, sel_initWithCharacters, (nint)utf16Ptr, length);
            return new NSString(newString);
        }
    }


    public static implicit operator string(NSString @string)
    {
        unsafe
        {
            byte* utf8Ptr = bytePtr_objc_msgSend(@string.Handle, sel_utf8String);
            return GetUtf8String(utf8Ptr);
        }
    }
}

[DebuggerDisplay("{DebuggerDisplay,nq}")]
internal readonly partial struct NSArray : IDisposable, IEquatable<NSArray>
{
    #region Selectors
    private static readonly Selector s_sel_count = "count";
    private static readonly Selector s_sel_objectAtIndex = "objectAtIndex:";
    #endregion

    public NSArray(nint handle) => Handle = handle;

    public NSArray()
    {
        ObjectiveCClass cls = new("NSArray");
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
