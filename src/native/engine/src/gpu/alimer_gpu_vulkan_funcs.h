// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

/* Functions that don't require an instance */
#ifndef VULKAN_GLOBAL_FUNCTION
#define VULKAN_GLOBAL_FUNCTION(name)
#endif

VULKAN_GLOBAL_FUNCTION(vkEnumerateInstanceVersion)
VULKAN_GLOBAL_FUNCTION(vkCreateInstance)
VULKAN_GLOBAL_FUNCTION(vkEnumerateInstanceExtensionProperties)
VULKAN_GLOBAL_FUNCTION(vkEnumerateInstanceLayerProperties)

/* Functions that require an instance but don't require a device*/
#ifndef VULKAN_INSTANCE_FUNCTION
#define VULKAN_INSTANCE_FUNCTION(name)
#endif

// Vulkan 1.0
VULKAN_INSTANCE_FUNCTION(vkGetDeviceProcAddr)
VULKAN_INSTANCE_FUNCTION(vkCreateDevice)
VULKAN_INSTANCE_FUNCTION(vkDestroyInstance)
VULKAN_INSTANCE_FUNCTION(vkEnumerateDeviceExtensionProperties)
VULKAN_INSTANCE_FUNCTION(vkEnumeratePhysicalDevices)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceFeatures)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceFormatProperties)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceImageFormatProperties)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceMemoryProperties)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceProperties)

// Vulkan 1.1
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceProperties2)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceFeatures2)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceMemoryProperties2)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceFormatProperties2)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties2)

// VK_KHR_surface
VULKAN_INSTANCE_FUNCTION(vkDestroySurfaceKHR)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR)

// VK_EXT_debug_utils
VULKAN_INSTANCE_FUNCTION(vkCreateDebugUtilsMessengerEXT)
VULKAN_INSTANCE_FUNCTION(vkDestroyDebugUtilsMessengerEXT)
VULKAN_INSTANCE_FUNCTION(vkCmdBeginDebugUtilsLabelEXT)
VULKAN_INSTANCE_FUNCTION(vkCmdEndDebugUtilsLabelEXT)
VULKAN_INSTANCE_FUNCTION(vkSetDebugUtilsObjectNameEXT)
VULKAN_INSTANCE_FUNCTION(vkCmdInsertDebugUtilsLabelEXT)

#if defined(VK_KHR_win32_surface)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceWin32PresentationSupportKHR)
VULKAN_INSTANCE_FUNCTION(vkCreateWin32SurfaceKHR)
#endif /* defined(VK_KHR_win32_surface) */

#if defined(VK_KHR_external_memory_win32)
VULKAN_INSTANCE_FUNCTION(vkGetMemoryWin32HandleKHR)
VULKAN_INSTANCE_FUNCTION(vkGetMemoryWin32HandlePropertiesKHR)
#endif /* defined(VK_KHR_external_memory_win32) */

#if defined(VK_EXT_metal_surface)
VULKAN_INSTANCE_FUNCTION(vkCreateMetalSurfaceEXT)
#endif /* defined(VK_EXT_metal_surface) */

#if defined(VK_KHR_wayland_surface)
VULKAN_INSTANCE_FUNCTION(vkCreateWaylandSurfaceKHR)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceWaylandPresentationSupportKHR)
#endif /* defined(VK_KHR_wayland_surface) */

#if defined(VK_KHR_xlib_surface)
VULKAN_INSTANCE_FUNCTION(vkCreateXlibSurfaceKHR)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceXlibPresentationSupportKHR)
#endif /* defined(VK_KHR_xlib_surface) */

#if defined(VK_KHR_xcb_surface)
VULKAN_INSTANCE_FUNCTION(vkCreateXcbSurfaceKHR)
VULKAN_INSTANCE_FUNCTION(vkGetPhysicalDeviceXcbPresentationSupportKHR)
#endif /* defined(VK_KHR_xcb_surface) */

#if defined(VK_KHR_android_surface)
VULKAN_INSTANCE_FUNCTION(vkCreateAndroidSurfaceKHR)
#endif /* defined(VK_KHR_android_surface) */

/* Functions that require a device */
#ifndef VULKAN_DEVICE_FUNCTION
#define VULKAN_DEVICE_FUNCTION(name)
#endif

// Vulkan 1.0
VULKAN_DEVICE_FUNCTION(vkAllocateCommandBuffers)
VULKAN_DEVICE_FUNCTION(vkAllocateDescriptorSets)
VULKAN_DEVICE_FUNCTION(vkAllocateMemory)
VULKAN_DEVICE_FUNCTION(vkBeginCommandBuffer)
VULKAN_DEVICE_FUNCTION(vkBindBufferMemory)
VULKAN_DEVICE_FUNCTION(vkBindImageMemory)
VULKAN_DEVICE_FUNCTION(vkCmdBeginRenderPass)
VULKAN_DEVICE_FUNCTION(vkCmdBindDescriptorSets)
VULKAN_DEVICE_FUNCTION(vkCmdBindIndexBuffer)
VULKAN_DEVICE_FUNCTION(vkCmdBindPipeline)
VULKAN_DEVICE_FUNCTION(vkCmdBindVertexBuffers)
VULKAN_DEVICE_FUNCTION(vkCmdBlitImage)
VULKAN_DEVICE_FUNCTION(vkCmdClearAttachments)
VULKAN_DEVICE_FUNCTION(vkCmdClearColorImage)
VULKAN_DEVICE_FUNCTION(vkCmdClearDepthStencilImage)
VULKAN_DEVICE_FUNCTION(vkCmdCopyBuffer)
VULKAN_DEVICE_FUNCTION(vkCmdCopyImage)
VULKAN_DEVICE_FUNCTION(vkCmdCopyBufferToImage)
VULKAN_DEVICE_FUNCTION(vkCmdCopyImageToBuffer)
VULKAN_DEVICE_FUNCTION(vkCmdDispatch)
VULKAN_DEVICE_FUNCTION(vkCmdDispatchIndirect)
VULKAN_DEVICE_FUNCTION(vkCmdDraw)
VULKAN_DEVICE_FUNCTION(vkCmdDrawIndexed)
VULKAN_DEVICE_FUNCTION(vkCmdDrawIndexedIndirect)
VULKAN_DEVICE_FUNCTION(vkCmdDrawIndirect)
VULKAN_DEVICE_FUNCTION(vkCmdEndRenderPass)
VULKAN_DEVICE_FUNCTION(vkCmdPipelineBarrier)
VULKAN_DEVICE_FUNCTION(vkCmdResolveImage)
VULKAN_DEVICE_FUNCTION(vkCmdSetBlendConstants)
VULKAN_DEVICE_FUNCTION(vkCmdSetDepthBias)
VULKAN_DEVICE_FUNCTION(vkCmdSetScissor)
VULKAN_DEVICE_FUNCTION(vkCmdSetStencilReference)
VULKAN_DEVICE_FUNCTION(vkCmdSetViewport)
VULKAN_DEVICE_FUNCTION(vkCreateBuffer)
VULKAN_DEVICE_FUNCTION(vkCreateCommandPool)
VULKAN_DEVICE_FUNCTION(vkCreateDescriptorPool)
VULKAN_DEVICE_FUNCTION(vkCreateDescriptorSetLayout)
VULKAN_DEVICE_FUNCTION(vkCreateFence)
VULKAN_DEVICE_FUNCTION(vkCreateFramebuffer)
VULKAN_DEVICE_FUNCTION(vkCreateComputePipelines)
VULKAN_DEVICE_FUNCTION(vkCreateGraphicsPipelines)
VULKAN_DEVICE_FUNCTION(vkCreateImage)
VULKAN_DEVICE_FUNCTION(vkCreateImageView)
VULKAN_DEVICE_FUNCTION(vkCreatePipelineCache)
VULKAN_DEVICE_FUNCTION(vkCreatePipelineLayout)
VULKAN_DEVICE_FUNCTION(vkCreateRenderPass)
VULKAN_DEVICE_FUNCTION(vkCreateSampler)
VULKAN_DEVICE_FUNCTION(vkCreateSemaphore)
VULKAN_DEVICE_FUNCTION(vkCreateShaderModule)
VULKAN_DEVICE_FUNCTION(vkDestroyBuffer)
VULKAN_DEVICE_FUNCTION(vkDestroyCommandPool)
VULKAN_DEVICE_FUNCTION(vkDestroyDescriptorPool)
VULKAN_DEVICE_FUNCTION(vkDestroyDescriptorSetLayout)
VULKAN_DEVICE_FUNCTION(vkDestroyDevice)
VULKAN_DEVICE_FUNCTION(vkDestroyFence)
VULKAN_DEVICE_FUNCTION(vkDestroyFramebuffer)
VULKAN_DEVICE_FUNCTION(vkDestroyImage)
VULKAN_DEVICE_FUNCTION(vkDestroyImageView)
VULKAN_DEVICE_FUNCTION(vkDestroyPipeline)
VULKAN_DEVICE_FUNCTION(vkDestroyPipelineCache)
VULKAN_DEVICE_FUNCTION(vkDestroyPipelineLayout)
VULKAN_DEVICE_FUNCTION(vkDestroyRenderPass)
VULKAN_DEVICE_FUNCTION(vkDestroySampler)
VULKAN_DEVICE_FUNCTION(vkDestroySemaphore)
VULKAN_DEVICE_FUNCTION(vkDestroyShaderModule)
VULKAN_DEVICE_FUNCTION(vkDeviceWaitIdle)
VULKAN_DEVICE_FUNCTION(vkEndCommandBuffer)
VULKAN_DEVICE_FUNCTION(vkFreeCommandBuffers)
VULKAN_DEVICE_FUNCTION(vkFreeMemory)
VULKAN_DEVICE_FUNCTION(vkGetDeviceQueue)
VULKAN_DEVICE_FUNCTION(vkGetPipelineCacheData)
VULKAN_DEVICE_FUNCTION(vkGetFenceStatus)
VULKAN_DEVICE_FUNCTION(vkGetBufferMemoryRequirements)
VULKAN_DEVICE_FUNCTION(vkGetImageMemoryRequirements)
VULKAN_DEVICE_FUNCTION(vkMapMemory)
VULKAN_DEVICE_FUNCTION(vkQueueSubmit)
VULKAN_DEVICE_FUNCTION(vkQueueWaitIdle)
VULKAN_DEVICE_FUNCTION(vkResetCommandBuffer)
VULKAN_DEVICE_FUNCTION(vkResetCommandPool)
VULKAN_DEVICE_FUNCTION(vkResetDescriptorPool)
VULKAN_DEVICE_FUNCTION(vkResetFences)
VULKAN_DEVICE_FUNCTION(vkUnmapMemory)
VULKAN_DEVICE_FUNCTION(vkUpdateDescriptorSets)
VULKAN_DEVICE_FUNCTION(vkWaitForFences)

// VK_KHR_swapchain
VULKAN_DEVICE_FUNCTION(vkAcquireNextImageKHR)
VULKAN_DEVICE_FUNCTION(vkCreateSwapchainKHR)
VULKAN_DEVICE_FUNCTION(vkDestroySwapchainKHR)
VULKAN_DEVICE_FUNCTION(vkQueuePresentKHR)
VULKAN_DEVICE_FUNCTION(vkGetSwapchainImagesKHR)

#if defined(VK_KHR_external_memory_fd)
VULKAN_DEVICE_FUNCTION(vkGetMemoryFdKHR)
//VULKAN_DEVICE_FUNCTION(vkGetMemoryFdPropertiesKHR)
#endif /* defined(VK_KHR_external_memory_fd) */

#if defined(VK_VERSION_1_2) 
VULKAN_DEVICE_FUNCTION(vkGetBufferDeviceAddress)
#endif /* defined(VK_VERSION_1_2) */

// Functions that require a device with 1.3 or VK_KHR_synchronization2
#if defined(VK_VERSION_1_3) || defined(VK_KHR_synchronization2)
VULKAN_DEVICE_FUNCTION(vkCmdPipelineBarrier2)
VULKAN_DEVICE_FUNCTION(vkCmdWriteTimestamp2)
VULKAN_DEVICE_FUNCTION(vkQueueSubmit2)
#endif /* defined(VK_VERSION_1_3) || defined(VK_KHR_synchronization2) */

#if defined(VK_EXT_conditional_rendering)
VULKAN_DEVICE_FUNCTION(vkCmdBeginConditionalRenderingEXT)
VULKAN_DEVICE_FUNCTION(vkCmdEndConditionalRenderingEXT)
#endif /* defined(VK_EXT_conditional_rendering) */

#if defined(VK_KHR_fragment_shading_rate)
VULKAN_DEVICE_FUNCTION(vkCmdSetFragmentShadingRateKHR)
#endif /* defined(VK_KHR_fragment_shading_rate) */

#if defined(VK_EXT_mesh_shader)
VULKAN_DEVICE_FUNCTION(vkCmdDrawMeshTasksEXT)
VULKAN_DEVICE_FUNCTION(vkCmdDrawMeshTasksIndirectCountEXT)
VULKAN_DEVICE_FUNCTION(vkCmdDrawMeshTasksIndirectEXT)
#endif /* defined(VK_EXT_mesh_shader) */

/* Redefine these every time you include this header! */
#undef VULKAN_GLOBAL_FUNCTION
#undef VULKAN_INSTANCE_FUNCTION
#undef VULKAN_DEVICE_FUNCTION
