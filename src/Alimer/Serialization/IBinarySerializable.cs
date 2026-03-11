// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Serialization;

/// <summary>
/// Base interface for types that can read from <see cref="ReadByteStream"/> and write to <see cref="WriteByteStream"/>.
/// </summary>
public interface IBinarySerializable<T> 
{
    /// <summary>
    /// Called when reading from a <see cref="ReadByteStream"/>.
    /// </summary>
    /// <param name="stream">The <see cref="ReadByteStream"/> to read from.</param>
    /// <returns>Implementations should read the necessary data from the stream and return an instance of the type.</returns>
    static abstract T Read(ref ReadByteStream stream, T? existingInstance);

    /// <summary>
    /// Called when writing to a <see cref="WriteByteStream"/>.
    /// </summary>
    /// <param name="stream">The <see cref="WriteByteStream"/> to write to.</param>
    /// <param name="value">The value to write.</param>
    static abstract void Write(ref WriteByteStream stream, T value);
}
