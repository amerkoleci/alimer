// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Audio;

namespace Alimer;

unsafe partial class AlimerApi
{
    #region Enums
    public enum VolumeUnit
    {
        Linear = 0,
        Decibels = 1,
    }
    #endregion

    #region Structs
    public struct AudioDevice
    {
        public AudioDeviceType deviceType;
        public nuint idSize;
        public void* id;
        public byte* name;
        public Bool32 isDefault;
    }

    public struct AudioConfig
    {
        /// <summary>
        /// Audio output channel count.
        /// </summary>
        public uint channelCount;
        /// <summary>
        /// Audio output sample rate.
        /// </summary>
        public uint sampleRate;
    }
    #endregion

    #region Handles
    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct AudioContext(nint handle) : IEquatable<AudioContext>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static AudioContext Null => new(0);
        public static implicit operator AudioContext(nint handle) => new(handle);
        public static implicit operator nint(AudioContext handle) => handle.Handle;

        public static bool operator ==(AudioContext left, AudioContext right) => left.Handle == right.Handle;
        public static bool operator !=(AudioContext left, AudioContext right) => left.Handle != right.Handle;
        public static bool operator ==(AudioContext left, nint right) => left.Handle == right;
        public static bool operator !=(AudioContext left, nint right) => left.Handle != right;
        public bool Equals(AudioContext other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is AudioContext handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(AudioContext)} [0x{Handle:X}]";
    }

    [DebuggerDisplay("{DebuggerDisplay,nq}")]
    public readonly partial struct AudioEngine(nint handle) : IEquatable<AudioEngine>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static AudioEngine Null => new(0);
        public static implicit operator AudioEngine(nint handle) => new(handle);
        public static implicit operator nint(AudioEngine handle) => handle.Handle;

        public static bool operator ==(AudioEngine left, AudioEngine right) => left.Handle == right.Handle;
        public static bool operator !=(AudioEngine left, AudioEngine right) => left.Handle != right.Handle;
        public static bool operator ==(AudioEngine left, nint right) => left.Handle == right;
        public static bool operator !=(AudioEngine left, nint right) => left.Handle != right;
        public bool Equals(AudioEngine other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is AudioEngine handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(AudioEngine)} [0x{Handle:X}]";
    }
    #endregion

    [LibraryImport(LibraryName)]
    public static partial AudioContext alimerAudioContextInit();

    [LibraryImport(LibraryName)]
    public static partial void alimerAudioContextDestroy(AudioContext context);

    [LibraryImport(LibraryName)]
    public static partial void alimerAudioContextEnumerateDevices(AudioContext context, delegate* unmanaged<AudioDevice*, nint, void> callback, nint userdata);

    [LibraryImport(LibraryName)]
    public static partial AudioEngine alimerAudioEngineCreate(AudioContext context, AudioConfig* config);
    [LibraryImport(LibraryName)]
    public static partial void alimerAudioEngineDestroy(AudioEngine engine);
    [LibraryImport(LibraryName)]
    public static partial void alimerAudioEngineStart(AudioEngine engine);
    [LibraryImport(LibraryName)]
    public static partial void alimerAudioEngineStop(AudioEngine engine);

    [LibraryImport(LibraryName)]
    public static partial AudioEngineState alimerAudioEngineGetState(AudioEngine engine);
    [LibraryImport(LibraryName)]
    public static partial float alimerAudioEngineGetMasterVolume(AudioEngine engine, VolumeUnit unit);
    [LibraryImport(LibraryName)]
    public static partial void alimerAudioEngineSetMasterVolume(AudioEngine engine, float value, VolumeUnit unit);
    [LibraryImport(LibraryName)]
    public static partial float alimerAudioEngineGetVolume(AudioEngine engine, VolumeUnit unit);
    [LibraryImport(LibraryName)]
    public static partial void alimerAudioEngineSetVolume(AudioEngine engine, float value, VolumeUnit unit);
    [LibraryImport(LibraryName)]
    public static partial int alimerAudioEngineGetChannelCount(AudioEngine engine);
    [LibraryImport(LibraryName)]
    public static partial int alimerAudioEngineGetSampleRate(AudioEngine engine);
}
