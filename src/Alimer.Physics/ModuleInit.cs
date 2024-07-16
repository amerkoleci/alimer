// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma warning disable CA2255
using System.Runtime.CompilerServices;
using Alimer.Engine;

namespace Alimer.Physics;

internal static class ModuleInit
{
    [ModuleInitializer]
    public static void Register()
    {
        EntityManager.RegisterSystemFactory<PhysicsSystem>();
    }
}
#pragma warning restore CA2255
