// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Samples;

public abstract class SampleBase : DisposableObject
{
    protected SampleBase(string name)
    {
        ArgumentNullException.ThrowIfNullOrEmpty(name, nameof(name));

        Name = name;
    }

    public string Name { get; }

    public virtual void Update(GameTime time)
    {
    }


    public virtual void Draw(CommandBuffer context, Texture outputTexture)
    {

    }
}
