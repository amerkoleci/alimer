// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Alimer
{
    /// <summary>
    /// Class describing the current running platform.
    /// </summary>
    public interface IRuntimePlatform
    {
        /// <summary>
        /// Gets the running platform type.
        /// </summary>
        PlatformType PlatformType { get; }

        /// <summary>
        /// Gets the running platform family.
        /// </summary>
        PlatformFamily PlatformFamily { get; }

        /// <summary>
		/// Gets the running framework description.
		/// </summary>
		string FrameworkDescription { get; }

        /// <summary>
		/// Get the operating system description.
		/// </summary>
		string OSDescription { get; }

        /// <summary>
		/// Returns the default directory name where the current application runs.
		/// </summary>
		string DefaultAppDirectory { get; }
    }
}
