// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Content;

public class ContentManager : DisposableObject, IContentManager
{
    private readonly IServiceRegistry _services;

    public ContentManager(IServiceRegistry services)
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));

        _services = services;
    }

    protected override void Dispose(bool disposing)
    {

    }
}
