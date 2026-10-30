// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.Serialization;

namespace Alimer.Engine;

public abstract partial class AsyncScript : ScriptComponent
{
    public CancellationToken CancellationToken => CancellationTokenSource?.Token ?? throw new InvalidOperationException();

    [IgnoreDataMember]
    internal CancellationTokenSource? CancellationTokenSource { get; set; }

    public abstract Task ExecuteAsync();
}
