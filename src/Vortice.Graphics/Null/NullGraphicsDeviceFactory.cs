// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.Generic;

namespace Vortice.Graphics.Null
{
    internal class NullGraphicsDeviceFactory : GraphicsDeviceFactory
    {
        public NullGraphicsDeviceFactory(ValidationMode validationMode)
            : base(validationMode)
        {
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="GraphicsDeviceNull" /> class.
        /// </summary>
        ~NullGraphicsDeviceFactory() => Dispose(disposing: false);

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
        }

        /// <inheritdoc />
        public override IReadOnlyList<PhysicalDevice> PhysicalDevices { get; }
    }
}
