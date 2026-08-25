// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Alimer.Graphics;

namespace Alimer;

internal static partial class ModuleInit
{
#pragma warning disable CA2255
    [ModuleInitializer]
    public static void Init()
    {
        Log.Init();
    }
#pragma warning restore CA2255
}
