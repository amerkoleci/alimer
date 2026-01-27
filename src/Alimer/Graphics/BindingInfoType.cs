// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum BindingInfoType : byte
{
    Undefined = 0,
    Buffer,
    Sampler,
    Texture,
    StorageTexture,
    AccelerationStructure
}
