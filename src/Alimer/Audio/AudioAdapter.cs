// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

public abstract class AudioAdapter
{
    protected AudioAdapter()
    {
    }

    public abstract string Name { get; }
    public abstract AudioAdapterType Type { get; }
    public abstract bool IsDefault { get; }
}
