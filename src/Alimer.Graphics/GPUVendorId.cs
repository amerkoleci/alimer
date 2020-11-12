// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Alimer.Graphics
{
    public enum KnownVendorId : uint
    {
        AMD = GPUVendorId.KnownVendorId_AMD,
        Intel = GPUVendorId.KnownVendorId_Intel,
        Nvidia = GPUVendorId.KnownVendorId_Nvidia,
        Microsoft = GPUVendorId.KnownVendorId_Microsoft,
        ARM = GPUVendorId.KnownVendorId_ARM,
        ImgTec = GPUVendorId.KnownVendorId_ImgTec,
        Qualcomm = GPUVendorId.KnownVendorId_Qualcomm,
    }

    [StructLayout(LayoutKind.Sequential)]
    public readonly struct GPUVendorId : IEquatable<GPUVendorId>
    {
        public const uint KnownVendorId_AMD = 0x1002;
        public const uint KnownVendorId_Intel = 0x8086;
        public const uint KnownVendorId_Nvidia = 0x10DE;
        public const uint KnownVendorId_Microsoft = 0x1414;
        public const uint KnownVendorId_ARM = 0x13B5;
        public const uint KnownVendorId_ImgTec = 0x1010;
        public const uint KnownVendorId_Qualcomm = 0x5143;

        /// <summary>
        /// Initializes a new instance of the <see cref="GPUVendorId" /> struct.
        /// </summary>
        /// <param name="value">The vendor id value.</param>
        public GPUVendorId(uint value)
        {
            Id = value;
        }

        public uint Id { get; }

        public KnownVendorId KnownVendor => (KnownVendorId)Id;

        /// <inheritdoc/>
        public override bool Equals(object? obj) => (obj is GPUVendorId other) && Equals(other);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Equals(GPUVendorId other) => Id == other.Id;

        /// <inheritdoc/>
        public override int GetHashCode() => Id.GetHashCode();

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator ==(GPUVendorId left, GPUVendorId right) => left.Equals(right);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator !=(GPUVendorId left, GPUVendorId right) => !left.Equals(right);

        /// <inheritdoc/>
        public override string ToString() => $"{KnownVendor} - {Id}";
    }
}
