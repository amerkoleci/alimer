// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Serialization;

/// <summary>
/// Result codes for serialization operations.
/// </summary>
public enum SerializationResult
{
    /// <summary>
    /// Operation completed successfully.
    /// </summary>
	Ok,
    /// <summary>
    /// A null value was encountered where one is not allowed.
    /// </summary>
	NullValue,
}
