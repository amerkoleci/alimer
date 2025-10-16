// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;

namespace Alimer.Platforms.Apple;

[DebuggerDisplay("{DebuggerDisplay,nq}")]
internal readonly partial struct Selector : IEquatable<Selector>
{
    public Selector(nint handle)
    {
        Handle = handle;
    }

    public Selector(string name)
    {
        Handle = ObjectiveC.sel_getUid(name);

        if (Handle == 0)
        {
            throw new InvalidOperationException($"Failed to get selector {name}!");
        }
    }

    public nint Handle { get; }
    public readonly bool IsNull => Handle == 0;
    public readonly bool IsNotNull => Handle != 0;

    public string Name
    {
        get => ObjectiveC.sel_getName(Handle);
    }

    public static Selector Null => new(0);
    public static implicit operator Selector(nint handle) => new(handle);
    public static implicit operator nint(Selector selector) => selector.Handle;
    public static implicit operator Selector(string value) => new(value);

    public static bool operator ==(Selector left, Selector right) => left.Handle == right.Handle;
    public static bool operator !=(Selector left, Selector right) => left.Handle != right.Handle;
    public static bool operator ==(Selector left, nint right) => left.Handle == right;
    public static bool operator !=(Selector left, nint right) => left.Handle != right;
    public bool Equals(Selector other) => Handle == other.Handle;
    /// <inheritdoc/>
    public override bool Equals([NotNullWhen(true)] object? obj) => obj is Selector handle && Equals(handle);
    /// <inheritdoc/>
    public override readonly int GetHashCode() => Handle.GetHashCode();
    private readonly string DebuggerDisplay => $"{nameof(Selector)} [0x{Handle:X}]";
}
