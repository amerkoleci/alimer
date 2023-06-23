// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.VGPU;

internal static class VGPUUtils
{
    public static VGPUBackend ToVGPU(this GraphicsBackendType backendType)
    {
        switch (backendType)
        {
            case GraphicsBackendType.Vulkan:
                return VGPUBackend.Vulkan;
            case GraphicsBackendType.D3D12:
                return VGPUBackend.D3D12;
            case GraphicsBackendType.D3D11:
                return VGPUBackend.D3D11;

            default:
            case GraphicsBackendType.Null:
            case GraphicsBackendType.Metal:
            case GraphicsBackendType.WebGPU:
                throw new InvalidOperationException();
        }
    }

    public static VGPUValidationMode ToVGPU(this ValidationMode value)
    {
        switch (value)
        {
            case ValidationMode.Disabled:
                return VGPUValidationMode.Disabled;
            case ValidationMode.Enabled:
                return VGPUValidationMode.Enabled;
            case ValidationMode.Verbose:
                return VGPUValidationMode.Verbose;
            case ValidationMode.GPU:
                return VGPUValidationMode.GPU;

            default:
                return VGPUValidationMode.Disabled;
        }
    }

    public static VGPUPowerPreference ToVGPU(this GpuPowerPreference value)
    {
        switch (value)
        {
            case GpuPowerPreference.Undefined:
                return VGPUPowerPreference.Undefined;
            case GpuPowerPreference.LowPower:
                return VGPUPowerPreference.LowPower;
            case GpuPowerPreference.HighPerformance:
                return VGPUPowerPreference.HighPerformance;

            default:
                return VGPUPowerPreference.Undefined;
        }
    }
}

