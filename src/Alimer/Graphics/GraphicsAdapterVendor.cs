// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

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

    /// Adapter vendor is Qualcomm
    GPUAdapterVendor_Qualcomm,

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
