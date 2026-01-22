// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

public abstract class KeyEventArgs : EventArgs
{

    /// <summary>
    /// Gets the key associated with this instance.
    /// </summary>
    public abstract Keys Key { get; }
}
