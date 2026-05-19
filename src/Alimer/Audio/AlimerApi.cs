// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Audio;

#pragma warning disable CS0649

namespace Alimer;

unsafe partial class AlimerApi
{
    #region Structs

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
    public readonly partial struct AudioDevice(nint handle) : IEquatable<AudioDevice>
    {
        public nint Handle { get; } = handle;
        public readonly bool IsNull => Handle == 0;
        public readonly bool IsNotNull => Handle != 0;

        public static AudioDevice Null => new(0);
        public static implicit operator AudioDevice(nint handle) => new(handle);
        public static implicit operator nint(AudioDevice handle) => handle.Handle;

        public static bool operator ==(AudioDevice left, AudioDevice right) => left.Handle == right.Handle;
        public static bool operator !=(AudioDevice left, AudioDevice right) => left.Handle != right.Handle;
        public static bool operator ==(AudioDevice left, nint right) => left.Handle == right;
        public static bool operator !=(AudioDevice left, nint right) => left.Handle != right;
        public bool Equals(AudioDevice other) => Handle == other.Handle;
        /// <inheritdoc/>
        public override bool Equals([NotNullWhen(true)] object? obj) => obj is AudioDevice handle && Equals(handle);
        /// <inheritdoc/>
        public override readonly int GetHashCode() => Handle.GetHashCode();
        private readonly string DebuggerDisplay => $"{nameof(AudioDevice)} [0x{Handle:X}]";
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
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerAudioInit();

    [LibraryImport(LibraryName)]
    public static partial void alimerAudioShutdown();

    [LibraryImport(LibraryName)]
    public static partial void alimerAudioEnumerateDevices(delegate* unmanaged<AudioDevice, nint, void> callback, nint userdata);

    [LibraryImport(LibraryName)]
    public static partial AudioDeviceType alimerAudioDeviceGetType(AudioDevice device);

    [LibraryImport(LibraryName)]
    public static partial byte* alimerAudioDeviceGetName(AudioDevice device);

    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerAudioDeviceIsDefault(AudioDevice device);

    [LibraryImport(LibraryName)]
    public static partial AudioEngine alimerAudioEngineCreate(AudioConfig* config);
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

    /* AudioClip */
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial nint alimerAudioClipCreate(string filepath);
    [LibraryImport(LibraryName)]
    public static partial nint alimerAudioClipCreateFromMemory(void* pData, nuint dataSize);
    [LibraryImport(LibraryName)]
    public static partial uint alimerAudioClipAddRef(nint clip);
    [LibraryImport(LibraryName)]
    public static partial uint alimerAudioClipRelease(nint clip);

    /* AudioSource */
    [LibraryImport(LibraryName)]
    public static partial nint alimerAudioSourceCreate(AudioEngine engine, nint clip);
    [LibraryImport(LibraryName)]
    public static partial uint alimerAudioSourceAddRef(nint source);
    [LibraryImport(LibraryName)]
    public static partial uint alimerAudioSourceRelease(nint source);
    [LibraryImport(LibraryName)]
    public static partial void alimerAudioSourcePlay(nint source);
    [LibraryImport(LibraryName)]
    public static partial void alimerAudioSourceStop(nint source);
    [LibraryImport(LibraryName)]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool alimerAudioSourceIsPlaying(nint source);

}
