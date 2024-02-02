// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;

namespace Alimer.Bindings.OpenAL;

[DebuggerDisplay("{DebuggerDisplay,nq}")]
public readonly partial struct ALCdevice : IEquatable<ALCdevice>
{
    public ALCdevice(nint handle) { Handle = handle; }
    public nint Handle { get; }
    public bool IsNull => Handle == 0;
    public bool IsNotNull => Handle != 0;
    public static ALCdevice Null => new(0);
    public static implicit operator ALCdevice(nint handle) => new(handle);
    public static implicit operator nint(ALCdevice handle) => handle.Handle;
    public static bool operator ==(ALCdevice left, ALCdevice right) => left.Handle == right.Handle;
    public static bool operator !=(ALCdevice left, ALCdevice right) => left.Handle != right.Handle;
    public static bool operator ==(ALCdevice left, nint right) => left.Handle == right;
    public static bool operator !=(ALCdevice left, nint right) => left.Handle != right;
    public bool Equals(ALCdevice other) => Handle == other.Handle;
    /// <inheritdoc/>
    public override bool Equals(object? obj) => obj is ALCdevice handle && Equals(handle);
    /// <inheritdoc/>
    public override int GetHashCode() => Handle.GetHashCode();
    private string DebuggerDisplay => $"{nameof(ALCdevice)} [0x{Handle.ToString("X")}]";
}
