// Copyright Â© Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

using System;

namespace TerraFX.Interop;

internal unsafe interface INativeGuid
{
    protected internal static abstract Guid* NativeGuid { get; }
}
