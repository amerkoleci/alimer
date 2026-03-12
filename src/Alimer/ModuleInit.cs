// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Alimer.Engine;

namespace Alimer;

internal static class ModuleInit
{
#pragma warning disable CA2255
    [ModuleInitializer]
#pragma warning restore CA2255
    public static void Register()
    {
        MetadataRegistry.RegisterObjectType<TransformComponent>();
        MetadataRegistry.RegisterObjectType<CameraComponent>();
        MetadataRegistry.RegisterObjectType<LightComponent>();
        MetadataRegistry.RegisterObjectType<MeshComponent>();
    }
}
