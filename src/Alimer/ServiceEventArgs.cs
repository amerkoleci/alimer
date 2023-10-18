// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

public class ServiceEventArgs(Type serviceType, object serviceInstance) : EventArgs
{
    public Type ServiceType { get; } = serviceType;
    public object Instance { get; } = serviceInstance;
}
