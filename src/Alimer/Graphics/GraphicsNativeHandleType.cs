// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum GraphicsNativeHandleType : uint
{
    Unknown = 0x00000000,
    SharedHandle = 0x00000001,

    /* Direct3D12 */
    DXGIFactory = 0x00020001,
    DXGIAdapter = 0x00020002,
    D3D12Device = 0x00020003,
    D3D12CommandQueue = 0x00020004,
    D3D12GraphicsCommandList = 0x00020005,
    D3D12Resource = 0x00020006,

    /* Vulkan */
    VkInstance = 0x00030001,
    VkPhysicalDevice = 0x00030002,
    VkDevice = 0x00030003,
    VkQueue = 0x00030004,
    VkCommandBuffer = 0x00030005,
    VkBuffer = 0x00030006,
    VkImage = 0x00030007,
    VkImageView = 0x00030008,
}
