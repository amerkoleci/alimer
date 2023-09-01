﻿// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Content;

namespace Alimer;

public interface IApplication
{
    //IServiceProvider Services { get; }

    IContentManager Content { get; }
}
