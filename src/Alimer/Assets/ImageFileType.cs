// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

/// <summary>
/// Enum describing format of an image from a file or memory. This is used to determine how to decode the image data.
/// </summary>
public enum ImageFileType
{
    /// <summary>Unknown image format.</summary>
    Unknown = 0,
    /// <summary>Windows/OS2 bitmap.</summary>
    Bmp,
    /// <summary>Portable Network Graphics, including the APNG animation extension.</summary>
    Png,
    /// <summary>Netpbm family: PBM, PGM, PPM and PAM, in both ASCII and binary encodings.</summary>
    Pnm,
    /// <summary>JPEG, in either JFIF or Exif framing.</summary>
    Jpeg,
    /// <summary>Graphics Interchange Format, including animated files.</summary>
    Gif,
    /// <summary>Windows icon or cursor, a directory of embedded BMP or PNG images.</summary>
    Ico,
    /// <summary>OpenEXR high dynamic range image.</summary>
    Exr,
    /// <summary>Radiance RGBE high dynamic range image (.hdr / .pic).</summary>
    Hdr,
    /// <summary>Truevision TGA. Has no leading signature, so detection relies on the file footer or the extension.</summary>
    Tga,
    /// <summary>Tagged Image File Format, little or big endian, classic or BigTIFF.</summary>
    Tiff,
    /// <summary>WebP, a RIFF container holding VP8, VP8L or VP8X payloads.</summary>
    Webp,
    /// <summary>Adobe Photoshop document (.psd) or large document (.psb).</summary>
    Psd,
    /// <summary>DirectDraw Surface, a GPU texture container for block-compressed and raw formats.</summary>
    Dds,
    /// <summary>Khronos Texture 1, a GPU texture container format.</summary>
    Ktx1,
    /// <summary>Khronos Texture 2, a GPU texture container format.</summary>
    Ktx2
}
