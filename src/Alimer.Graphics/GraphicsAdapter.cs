// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Alimer.Graphics
{
    public abstract class GraphicsAdapter 
    {
        /// <summary>
        /// Create new instance of the <see cref="GraphicsAdapter" /> class.
        /// </summary>
        protected GraphicsAdapter()
        {
        }

        public abstract int VendorId { get; }
        public abstract int DeviceId { get; }
        public abstract string Name { get; }
    }
}
