// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Graphics.Vulkan
{
    internal unsafe class ResourceMemoryAllocator : IDisposable
    {
        private readonly List<SingleTypeAllocator> _allocatorsPerType;

        public ResourceMemoryAllocator(GraphicsDeviceVulkan device)
        {
            Device = device;

            vkGetPhysicalDeviceMemoryProperties(device.PhysicalDevice, out VkPhysicalDeviceMemoryProperties memoryProperties);

            _allocatorsPerType = new List<SingleTypeAllocator>((int)memoryProperties.memoryTypeCount);
            for (uint i = 0; i < memoryProperties.memoryTypeCount; i++)
            {
                VkMemoryType memoryType = memoryProperties.GetMemoryType(i);
                _allocatorsPerType.Add(new SingleTypeAllocator(device, i, memoryProperties.GetMemoryHeap(memoryType.heapIndex).size));
            }
        }

        public GraphicsDeviceVulkan Device { get; }

        /// <inheritdoc />
        public void Dispose()
        {
        }

        class SingleTypeAllocator
        {
            private readonly GraphicsDeviceVulkan _device;
            private readonly uint _memoryTypeIndex;
            private readonly ulong _memoryHeapSize;

            public SingleTypeAllocator(GraphicsDeviceVulkan device, uint memoryTypeIndex, ulong memoryHeapSize)
            {
                _device = device;
                _memoryTypeIndex = memoryTypeIndex;
                _memoryHeapSize = memoryHeapSize;
            }
        }
    }
}
