// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;

namespace Alimer.Bindings.OpenAL;

[DebuggerDisplay("{DebuggerDisplay,nq}")]
public readonly partial struct ALCcontext : IEquatable<ALCcontext>
{
    public ALCcontext(nint handle) { Handle = handle; }
    public nint Handle { get; }
    public bool IsNull => Handle == 0;
    public bool IsNotNull => Handle != 0;
    public static ALCcontext Null => new(0);
    public static implicit operator ALCcontext(nint handle) => new(handle);
    public static implicit operator nint(ALCcontext handle) => handle.Handle;
    public static bool operator ==(ALCcontext left, ALCcontext right) => left.Handle == right.Handle;
    public static bool operator !=(ALCcontext left, ALCcontext right) => left.Handle != right.Handle;
    public static bool operator ==(ALCcontext left, nint right) => left.Handle == right;
    public static bool operator !=(ALCcontext left, nint right) => left.Handle != right;
    public bool Equals(ALCcontext other) => Handle == other.Handle;
    /// <inheritdoc/>
    public override bool Equals(object? obj) => obj is ALCcontext handle && Equals(handle);
    /// <inheritdoc/>
    public override int GetHashCode() => Handle.GetHashCode();
    private string DebuggerDisplay => $"{nameof(ALCcontext)} [0x{Handle.ToString("X")}]";
}
