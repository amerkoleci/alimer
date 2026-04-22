// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Enum defining the vendor of <see cref="GraphicsAdapter"/>
/// </summary>
public enum GraphicsAdapterVendor
{
    /// <summary>
    /// Adapter vendor is unknown
    /// </summary>
    Unknown = 0,
    /// <summary>
    /// Adapter vendor is NVIDIA
    /// </summary>
    NVIDIA,
    /// <summary>
    /// Adapter vendor is AMD
    /// </summary>
    AMD,
    /// <summary>
    /// Adapter vendor is Intel
    /// </summary>
    Intel,
    /// <summary>
    /// Adapter vendor is ARM
    /// </summary>
    ARM,
    /// <summary>
    /// Adapter vendor is Qualcomm
    /// </summary>
    Qualcomm,
    /// <summary>
    /// Adapter vendor is Imagination Technologies
    /// </summary>
    ImgTech,
    /// <summary>
    /// Adapter vendor is Microsoft (software rasterizer)
    /// </summary>
    MSFT,
    /// <summary>
    /// Adapter vendor is Apple
    /// </summary>
    Apple,
    /// <summary>
    /// Adapter vendor is Mesa (software rasterizer)
    /// </summary>
    Mesa,
    /// <summary>
    /// Adapter vendor is Broadcom (Raspberry Pi)
    /// </summary>
    Broadcom,
}

internal enum KnownGPUAdapterVendor
{
    AMD = 0x01002,
    NVIDIA = 0x010DE,
    INTEL = 0x08086,
    ARM = 0x013B5,
    QUALCOMM = 0x05143,
    IMGTECH = 0x01010,
    MSFT = 0x01414,
    APPLE = 0x0106B,
    MESA = 0x10005,
    BROADCOM = 0x014e4
}
