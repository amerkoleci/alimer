// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma warning disable CA2255
using System.Runtime.CompilerServices;

namespace Alimer.Engine;

internal static class ModuleInit
{
    [ModuleInitializer]
    public static void Register()
    {
        EntityManager.RegisterSystemFactory<TransformSystem>();
        EntityManager.RegisterSystemFactory<CameraSystem>();
        EntityManager.RegisterSystemFactory((services) => new RenderSystem(services));
    }
}
#pragma warning restore CA2255
