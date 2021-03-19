// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

using System;

namespace Vortice.Content
{
    /// <summary>
    /// Provides a service to load runtime content data.
    /// </summary>
    public partial class ContentManager : IContentManager
    {
        public ContentManager(IServiceProvider services, IFileProvider fileProvider)
        {
            Guard.AssertNotNull(services);
            Guard.AssertNotNull(fileProvider);

            Services = services;
            FileProvider = fileProvider;
        }

        public IServiceProvider Services { get; }

        public IFileProvider FileProvider { get; }
    }
}
