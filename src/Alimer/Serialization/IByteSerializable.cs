// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Serialization;

public interface IByteSerializable
{
    static abstract object ReadObject(ref ReadByteStream stream);
    static abstract void WriteObject(ref WriteByteStream stream, object value);
}
