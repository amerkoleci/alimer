// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

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
        public override GraphicsSurface CreateSurface(in SurfaceSource source)
        {
            return null;
        }

        /// <inheritdoc />
        public override GraphicsAdapter? RequestAdapter(GPUPowerPreference powerPreference)
        {
            return null;
        }
    }
}
