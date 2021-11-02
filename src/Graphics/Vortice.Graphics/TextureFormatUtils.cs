// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics
{
    public static class TextureFormatUtils
    {
        /// <summary>
        /// Check if the format has a depth component.
        /// </summary>
        /// <param name="format">The <see cref="TextureFormat"/> to check.</param>
        /// <returns>True if format has depth component, false otherwise.</returns>
        public static bool IsDepthFormat(this TextureFormat format)
        {
            switch (format)
            {
                case TextureFormat.Depth16UNorm:
                case TextureFormat.Depth32Float:
                case TextureFormat.Depth24UNormStencil8:
                case TextureFormat.Depth32FloatStencil8:
                    return true;

                default:
                    return false;
            }
        }

        /// <summary>
        /// Check if the format has a stencil component.
        /// </summary>
        /// <param name="format">The <see cref="TextureFormat"/> to check.</param>
        /// <returns>True if format has stencil component, false otherwise.</returns>
        public static bool IsStencilFormat(this TextureFormat format)
        {
            switch (format)
            {
                case TextureFormat.Depth24UNormStencil8:
                case TextureFormat.Depth32FloatStencil8:
                    return true;

                default:
                    return false;
            }
        }

        /// <summary>
        /// Check if the format has depth or stencil components.
        /// </summary>
        /// <param name="format">The <see cref="TextureFormat"/> to check.</param>
        /// <returns>True if format has depth or stencil component, false otherwise.</returns>
        public static bool IsDepthStencilFormat(this TextureFormat format)
        {
            return IsDepthFormat(format) || IsStencilFormat(format);
        }

        /// <summary>
        /// Check if the format is a compressed format.
        /// </summary>
        /// <param name="format"></param>
        /// <returns></returns>
        public static bool IsBlockCompressedFormat(this TextureFormat format)
        {
            switch (format)
            {
                case TextureFormat.BC1RGBAUNorm:
                case TextureFormat.BC1RGBAUNormSrgb:
                case TextureFormat.BC2RGBAUNorm:
                case TextureFormat.BC2RGBAUNormSrgb:
                case TextureFormat.BC3RGBAUNorm:
                case TextureFormat.BC3RGBAUNormSrgb:
                case TextureFormat.BC4RUNorm:
                case TextureFormat.BC4RSNorm:
                case TextureFormat.BC5RGUNorm:
                case TextureFormat.BC5RGSNorm:
                case TextureFormat.BC6HRGBUFloat:
                case TextureFormat.BC6HRGBFloat:
                case TextureFormat.BC7RGBAUNorm:
                case TextureFormat.BC7RGBAUNormSrgb:
                    return true;

                default:
                    return false;
            }
        }
    }
}
