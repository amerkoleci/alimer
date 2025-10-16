// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;

namespace Alimer.Graphics;

public abstract class CopyContext
{
    protected bool _hasLabel;

    protected CopyContext()
    {
    }

    /// <summary>
    /// Get the <see cref="QueueType"/> object that will execute this context.
    /// </summary>
    public virtual QueueType QueueType => QueueType.Copy;

    /// <summary>
    /// Get the <see cref="GraphicsDevice"/> object that created this object.
    /// </summary>
    public abstract GraphicsDevice Device { get; }

    /// <summary>
    /// Flush existing commands to the GPU and optinally wait for completion.
    /// </summary>
    /// <param name="waitForCompletion"></param>
    public abstract void Flush(bool waitForCompletion = false);

    public abstract void PushDebugGroup(Utf8ReadOnlyString groupLabel);
    public abstract void PopDebugGroup();
    public abstract void InsertDebugMarker(string debugLabel);

    public IDisposable PushScopedDebugGroup(Utf8ReadOnlyString groupLabel)
    {
        PushDebugGroup(groupLabel);
        return new ScopedDebugGroup(this);
    }

    #region Nested
    readonly struct ScopedDebugGroup(CopyContext context) : IDisposable
    {
        private readonly CopyContext _context = context;

        public void Dispose() => _context.PopDebugGroup();
    }
    #endregion
}
