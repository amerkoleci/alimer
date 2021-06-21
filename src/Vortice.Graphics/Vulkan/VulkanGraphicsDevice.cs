// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Graphics.Vulkan
{
    public sealed unsafe class VulkanGraphicsDevice : GraphicsDevice
    {
        private readonly GraphicsDeviceCaps _caps;

        internal VulkanGraphicsDevice(VkPhysicalDevice physicalDevice)
        {
            VkDeviceCreateInfo createInfo = new VkDeviceCreateInfo
            {
                sType = VkStructureType.DeviceCreateInfo
            };

            vkCreateDevice(physicalDevice, &createInfo, null, out VkDevice device);
            NativeDevice = device;
            PhysicalDevice = physicalDevice;

            vkGetPhysicalDeviceProperties(PhysicalDevice, out VkPhysicalDeviceProperties properties);

            GPUAdapterType adapterType = GPUAdapterType.Unknown;
            switch (properties.deviceType)
            {
                case VkPhysicalDeviceType.IntegratedGpu:
                    adapterType = GPUAdapterType.IntegratedGPU;
                    break;

                case VkPhysicalDeviceType.DiscreteGpu:
                    adapterType = GPUAdapterType.DiscreteGPU;
                    break;

                case VkPhysicalDeviceType.Cpu:
                    adapterType = GPUAdapterType.CPU;
                    break;

                default:
                    adapterType = GPUAdapterType.Unknown;
                    break;
            }


            _caps = new GraphicsDeviceCaps()
            {
                BackendType = GraphicsBackend.Vulkan,
                VendorId = new GPUVendorId(properties.vendorID),
                AdapterId = properties.deviceID,
                AdapterType = adapterType,
                AdapterName = properties.GetDeviceName(),
                Features = new GraphicsDeviceFeatures
                {
                    IndependentBlend = true,
                    ComputeShader = true,
                    TessellationShader = true,
                    MultiViewport = true,
                    IndexUInt32 = true,
                    MultiDrawIndirect = true,
                    FillModeNonSolid = true,
                    SamplerAnisotropy = true,
                    TextureCompressionETC2 = false,
                    TextureCompressionASTC_LDR = false,
                    TextureCompressionBC = true,
                    TextureCubeArray = true,
                    Raytracing = false
                },
                Limits = new GraphicsDeviceLimits
                {
                    MaxVertexAttributes = 16,
                    MaxVertexBindings = 16,
                    MaxVertexAttributeOffset = 2047,
                    MaxVertexBindingStride = 2048,
                    //MaxTextureDimension1D = RequestTexture1DUDimension,
                    //MaxTextureDimension2D = RequestTexture2DUOrVDimension,
                    //MaxTextureDimension3D = RequestTexture3DUVOrWDimension,
                    //MaxTextureDimensionCube = RequestTextureCubeDimension,
                    //MaxTextureArrayLayers = RequestTexture2DArrayAxisDimension,
                    //MaxColorAttachments = SimultaneousRenderTargetCount,
                    //MaxUniformBufferRange = RequestConstantBufferElementCount * 16,
                    MaxStorageBufferRange = uint.MaxValue,
                    MinUniformBufferOffsetAlignment = 256u,
                    MinStorageBufferOffsetAlignment = 16u,
                    //MaxSamplerAnisotropy = MaxMaxAnisotropy,
                    //MaxViewports = ViewportAndScissorRectObjectCountPerPipeline,
                    //MaxViewportWidth = ViewportBoundsMax,
                    //MaxViewportHeight = ViewportBoundsMax,
                    //MaxTessellationPatchSize = InputAssemblerPatchMaxControlPointCount,
                    //MaxComputeSharedMemorySize = ComputeShaderThreadLocalTempRegisterPool,
                    //MaxComputeWorkGroupCountX = ComputeShaderDispatchMaxThreadGroupsPerDimension,
                    //MaxComputeWorkGroupCountY = ComputeShaderDispatchMaxThreadGroupsPerDimension,
                    //MaxComputeWorkGroupCountZ = ComputeShaderDispatchMaxThreadGroupsPerDimension,
                    //MaxComputeWorkGroupInvocations = ComputeShaderThreadGroupMaxThreadsPerGroup,
                    //MaxComputeWorkGroupSizeX = ComputeShaderThreadGroupMaxX,
                    //MaxComputeWorkGroupSizeY = ComputeShaderThreadGroupMaxY,
                    //MaxComputeWorkGroupSizeZ = ComputeShaderThreadGroupMaxZ,
                }
            };
        }

        public VkDevice NativeDevice { get; }
        public VkPhysicalDevice PhysicalDevice { get; }

        /// <inheritdoc />
        public override GraphicsDeviceCaps Capabilities => _caps;

        /// <summary>
        /// Finalizes an instance of the <see cref="VulkanGraphicsDevice" /> class.
        /// </summary>
        ~VulkanGraphicsDevice() => Dispose(disposing: false);


        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
        }

        /// <inheritdoc />
        protected override Texture CreateTextureCore(in TextureDescriptor descriptor) => new VulkanTexture(this, descriptor);
    }
}
