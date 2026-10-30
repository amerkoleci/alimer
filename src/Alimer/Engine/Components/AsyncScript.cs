// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Engine;

public abstract partial class AsyncScript : ScriptComponent
{
    public CancellationToken CancellationToken => CancellationTokenSource?.Token ?? throw new InvalidOperationException();

    internal CancellationTokenSource? CancellationTokenSource { get; set; }

    public abstract Task ExecuteAsync();
}
