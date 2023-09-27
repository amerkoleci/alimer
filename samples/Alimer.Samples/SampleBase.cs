// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Samples;

public abstract class SampleBase : IDisposable
{
    protected SampleBase(string name)
    {
        Guard.IsNotNullOrEmpty(name, nameof(name));

        Name = name;
    }

    public string Name { get; }

    /// <summary>
    /// Gets or sets the disposables.
    /// </summary>
    /// <value>The disposables.</value>
    protected DisposeCollector? DisposeCollector { get; set; }

    public virtual void Dispose()
    {
        DisposeCollector?.Dispose();
        DisposeCollector = null;
    }

    public virtual void Draw(RenderContext context, Texture outputTexture)
    {

    }

    /// <summary>
    /// Adds a disposable object to the list of the objects to dispose.
    /// </summary>
    /// <param name="disposable">To dispose.</param>
    protected internal T ToDispose<T>(T disposable)
        where T : IDisposable
    {
        Guard.IsNotNull(disposable, nameof(disposable));

        DisposeCollector ??= new DisposeCollector();
        return DisposeCollector.Collect(disposable);
    }

    /// <summary>
    /// Dispose a disposable object and set the reference to null. Removes this object from the ToDispose list.
    /// </summary>
    /// <param name="disposable">Object to dispose.</param>
    protected internal void RemoveAndDispose<T>(ref T? disposable)
        where T : IDisposable
    {
        DisposeCollector?.RemoveAndDispose(ref disposable);
    }

    /// <summary>
    /// Removes a disposable object to the list of the objects to dispose.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="disposable">To dispose.</param>
    protected internal void RemoveToDispose<T>(T disposable)
        where T : IDisposable
    {
        Guard.IsNotNull(disposable, nameof(disposable));

        DisposeCollector?.Remove(disposable);
    }
}
