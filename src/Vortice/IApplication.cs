﻿// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;

namespace Vortice
{
    public interface IApplication
    {
        IReadOnlyList<IWindow> Windows { get; }
    }
}