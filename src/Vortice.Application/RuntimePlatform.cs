// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Vortice
{
    /// <summary>
    /// Class describing the current running platform.
    /// </summary>
    public static partial class RuntimePlatform
    {
        /// <summary>
        /// Gets the running platform type.
        /// </summary>
        public static PlatformType PlatformType { get; }

        /// <summary>
        /// Gets the running platform family.
        /// </summary>
        public static PlatformFamily PlatformFamily { get; }

        /// <summary>
        /// Gets the running framework description.
        /// </summary>
        public static string? FrameworkDescription { get; }

        /// <summary>
        /// Get the operating system description.
        /// </summary>
        public static string? OSDescription { get; }

        /// <summary>
        /// Returns the default directory name where the current application runs.
        /// </summary>
        public static string? DefaultAppDirectory { get; }
    }
}
