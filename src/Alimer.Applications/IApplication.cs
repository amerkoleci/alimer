// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Content;

namespace Alimer;

public interface IApplication
{
    IServiceRegistry Services { get; }

    IContentManager Content { get; }

    /// <summary>
    /// Gets a value indicating whether application is running.
    /// </summary>
    bool IsRunning { get; }

    /// <summary>
    /// Gets a value indicating whether application is exiting.
    /// </summary>
    bool IsExiting { get; }
}
