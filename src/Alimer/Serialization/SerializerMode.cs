// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Serialization;

/// <summary>
/// Serialization mode used during serialization.
/// </summary>
public enum SerializerMode
{
    /// <summary>
    /// Reads the data from the stream.
    /// </summary>
    Read,

    /// <summary>
    /// Writes the data to the stream.
    /// </summary>
    Write
}
