// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Bindings.OpenAL;

public enum AlcError
{
    NoError = 0,
    InvalidDevice = 0xA001,
    InvalidContext = 0xA002,
    InvalidEnum = 0xA003,
    InvalidValue = 0xA004,
    OutOfMemory = 0xA005,
}

public enum AlcGetInteger
{
    /// <summary>
    /// Runtime ALC major version.
    /// </summary>
    MajorVersion = 0x1000,
    /// <summary>
    /// Runtime ALC minor version.
    /// </summary>
    MinorVersion = 0x1001,
    /// <summary>
    /// Context attribute list size.
    /// </summary>
    AttributesSize = 0x1002,

    Frequency = 0x1007,
    Refresh = 0x1008,
    Sync = 0x1009,
    MonoSources = 0x1010,
    StereoSources = 0x1011,

    /// <summary>
    /// ALC_EXT_CAPTURE - Number of sample frames available for capture.
    /// </summary>
    CaptureSamples = 0x312,
}

public enum AlcGetString
{
    /// <summary>
    /// Context attribute list properties.
    /// </summary>
    AllAttributes = 0x1003,
    /// <summary>
    /// String for the default device specifier.
    /// </summary>
    DefaultDeviceSpecifier = 0x1004,
    /// <summary>
    /// Device specifier string.
    ///
    /// If device handle is NULL, it is instead a null-character separated list of strings of known device specifiers (list ends with an empty string).
    /// </summary>
    DeviceSpecifier = 0x1005,
    /// <summary>
    /// String for space-separated list of ALC extensions.
    /// </summary>
    Extensions = 0x1006,

    /// <summary>
    /// ALC_EXT_CAPTURE - Capture device specifier string.
    /// </summary>
    CaptureDeviceSpecifier = 0x310,

    /// <summary>
    /// ALC_EXT_CAPTURE - String for the default capture device specifier.
    /// </summary>
    CaptureDefaultDeviceSpecifier = 0x311,

    /// <summary>
    /// ALC_ENUMERATE_ALL_EXT - Device's extended specifier string.
    /// </summary>
    AllDevicesSpecifier = 0x1013,
}

public enum AlGetString
{
    /// <summary>
    /// Context string: Vendor name.
    /// </summary>
    Vendor = 0xB001,
    /// <summary>
    /// Context string: Version.
    /// </summary>
    Version = 0xB002,
    /// <summary>
    /// Context string: Renderer name.
    /// </summary>
    Renderer = 0xB003,
    /// <summary>
    /// Context string: Space-separated extension list.
    /// </summary>
    Extensions = 0xB004
}

public enum AlDistanceModel
{
    /// <summary>
    /// Bypasses all distance attenuation calculation for all Sources.
    /// </summary>
    None = 0,

    /// <summary>
    /// InverseDistance is equivalent to the IASIG I3DL2 model with the exception that SourceFloat.ReferenceDistance
    /// does not imply any clamping.
    /// </summary>
    InverseDistance = 0xD001,

    /// <summary>
    /// InverseDistanceClamped is the IASIG I3DL2 model, with SourceFloat.ReferenceDistance indicating both the
    /// reference distance and the distance below which gain will be clamped.
    /// </summary>
    InverseDistanceClamped = 0xD002,

    /// <summary>
    /// AL_EXT_LINEAR_DISTANCE extension.
    /// </summary>
    LinearDistance = 0xD003,

    /// <summary>
    /// AL_EXT_LINEAR_DISTANCE extension.
    /// </summary>
    LinearDistanceClamped = 0xD004,

    /// <summary>
    /// AL_EXT_EXPONENT_DISTANCE extension.
    /// </summary>
    ExponentDistance = 0xD005,

    /// <summary>
    /// AL_EXT_EXPONENT_DISTANCE extension.
    /// </summary>
    ExponentDistanceClamped = 0xD006
}
