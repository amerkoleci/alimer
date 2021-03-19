// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

namespace Vortice.Content
{
    /// <summary>
    /// Provides a service to load runtime content data.
    /// </summary>
    public interface IContentManager
    {
        public IFileProvider FileProvider { get; }
    }
}
