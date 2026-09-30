// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

/// <summary>
/// Represents errors that occur in the Audio library.
/// </summary>
public class AudioException : Exception
{
    /// <summary>
    /// Constructs a new instance of the <see cref="AudioException"/> class.
    /// </summary>
    public AudioException()
    {
    }

    /// <summary>
    /// Constructs a new instance of the <see cref="AudioException"/> class with the given message.
    /// </summary>
    /// <param name="message">The exception message.</param>
    public AudioException(string message) : base(message)
    {
    }

    /// <summary>
    /// Constructs a new instance of the <see cref="AudioException"/> class with the given message and inner exception.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    public AudioException(string message, Exception innerException) : base(message, innerException)
    {
    }
}
