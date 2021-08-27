// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_RHI_VULKAN) 
#include "RHI.h"
#include "Window.h"
#include "Core/Log.h"
#include "PlatformInclude.h"
#include "volk.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "spirv_reflect.h"
#include <optional>

/// Helper macro to test the result of Vulkan calls which can return an error.
#define VK_CHECK(x) \
	do \
	{ \
		VkResult err = x; \
		if (err) \
		{ \
			LOGE("Detected Vulkan error: {}", alimer::rhi::ToString(err)); \
		} \
	} while (0)

#define VK_LOG_ERROR(result, message) LOGE("Vulkan: {}, error: {}", message, alimer::rhi::ToString(result));

namespace alimer::rhi
{
    [[nodiscard]] constexpr const char* ToString(VkResult result)
    {
        switch (result)
        {
#define STR(r)   \
	case VK_##r: \
		return #r
            STR(SUCCESS);
            STR(NOT_READY);
            STR(TIMEOUT);
            STR(EVENT_SET);
            STR(EVENT_RESET);
            STR(INCOMPLETE);
            STR(ERROR_OUT_OF_HOST_MEMORY);
            STR(ERROR_OUT_OF_DEVICE_MEMORY);
            STR(ERROR_INITIALIZATION_FAILED);
            STR(ERROR_DEVICE_LOST);
            STR(ERROR_MEMORY_MAP_FAILED);
            STR(ERROR_LAYER_NOT_PRESENT);
            STR(ERROR_EXTENSION_NOT_PRESENT);
            STR(ERROR_FEATURE_NOT_PRESENT);
            STR(ERROR_INCOMPATIBLE_DRIVER);
            STR(ERROR_TOO_MANY_OBJECTS);
            STR(ERROR_FORMAT_NOT_SUPPORTED);
            STR(ERROR_SURFACE_LOST_KHR);
            STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
            STR(SUBOPTIMAL_KHR);
            STR(ERROR_OUT_OF_DATE_KHR);
            STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
            STR(ERROR_VALIDATION_FAILED_EXT);
            STR(ERROR_INVALID_SHADER_NV);
#undef STR
        default:
            return "UNKNOWN_ERROR";
        }
    }

    namespace
    {
        const char* kDeviceTypes[] = {
            "Other",
            "IntegratedGPU",
            "DiscreteGPU",
            "VirtualGPU",
            "CPU"
        };

        VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData)
        {
            std::string messageTypeStr = "General";

            if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
                messageTypeStr = "Validation";
            else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
                messageTypeStr = "Performance";

            // Log debug messge
            if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                LOGW("Vulkan - {}: {}", messageTypeStr.c_str(), pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                LOGE("Vulkan - {}: {}", messageTypeStr.c_str(), pCallbackData->pMessage);
            }

            return VK_FALSE;
        }

        bool ValidateLayers(const std::vector<const char*>& required,
            const std::vector<VkLayerProperties>& available)
        {
            for (auto layer : required)
            {
                bool found = false;
                for (auto& available_layer : available)
                {
                    if (strcmp(available_layer.layerName, layer) == 0)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    LOGW("Validation Layer '{}' not found", layer);
                    return false;
                }
            }

            return true;
        }

        std::vector<const char*> GetOptimalValidationLayers(const std::vector<VkLayerProperties>& supported_instance_layers)
        {
            std::vector<std::vector<const char*>> validation_layer_priority_list =
            {
                // The preferred validation layer is "VK_LAYER_KHRONOS_validation"
                {"VK_LAYER_KHRONOS_validation"},

                // Otherwise we fallback to using the LunarG meta layer
                {"VK_LAYER_LUNARG_standard_validation"},

                // Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
                {
                    "VK_LAYER_GOOGLE_threading",
                    "VK_LAYER_LUNARG_parameter_validation",
                    "VK_LAYER_LUNARG_object_tracker",
                    "VK_LAYER_LUNARG_core_validation",
                    "VK_LAYER_GOOGLE_unique_objects",
                },

                // Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
                {"VK_LAYER_LUNARG_core_validation"}
            };

            for (auto& validation_layers : validation_layer_priority_list)
            {
                if (ValidateLayers(validation_layers, supported_instance_layers))
                {
                    return validation_layers;
                }

                LOGW("Couldn't enable validation layers (see log for error) - falling back");
            }

            // Else return nothing
            return {};
        }

        struct PhysicalDeviceExtensions
        {
            bool swapchain;
            bool depth_clip_enable;
            bool memory_budget;
            bool performance_query;
            bool host_query_reset;
            bool sampler_mirror_clamp_to_edge;
            bool spirv_1_4;
            bool buffer_device_address;
            bool deferred_host_operations;
            bool descriptor_indexing;
            bool accelerationStructure;
            bool raytracingPipeline;
            bool rayQuery;
            bool create_renderpass2;
            bool fragment_shading_rate;
            bool NV_mesh_shader;
            bool win32_full_screen_exclusive;
        };

        struct QueueFamilyIndices {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            bool isComplete() {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        };

        PhysicalDeviceExtensions QueryPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice)
        {
            uint32_t count = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr));
            std::vector<VkExtensionProperties> vk_extensions(count);
            VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, vk_extensions.data()));

            PhysicalDeviceExtensions extensions{};

            for (uint32_t i = 0; i < count; ++i)
            {
                //LOG_INFO("Extension: {}", vk_extensions[i].extensionName);

                if (strcmp(vk_extensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                    extensions.swapchain = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME) == 0) {
                    extensions.depth_clip_enable = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0) {
                    extensions.memory_budget = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME) == 0) {
                    extensions.performance_query = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME) == 0) {
                    extensions.host_query_reset = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME) == 0) {
                    extensions.sampler_mirror_clamp_to_edge = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_SPIRV_1_4_EXTENSION_NAME) == 0) {
                    extensions.spirv_1_4 = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) == 0) {
                    extensions.buffer_device_address = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0) {
                    extensions.deferred_host_operations = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) == 0) {
                    extensions.descriptor_indexing = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0) {
                    extensions.accelerationStructure = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0) {
                    extensions.raytracingPipeline = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0) {
                    extensions.rayQuery = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) == 0) {
                    extensions.create_renderpass2 = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME) == 0) {
                    extensions.fragment_shading_rate = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_NV_MESH_SHADER_EXTENSION_NAME) == 0) {
                    extensions.NV_mesh_shader = true;
                }
                else if (strcmp(vk_extensions[i].extensionName, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) == 0) {
                    extensions.win32_full_screen_exclusive = true;
                }
            }

            // We run on vulkan 1.1 or higher.
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(physicalDevice, &props);

            // Promoted in 1.2
            if (props.apiVersion >= VK_API_VERSION_1_2)
            {
                extensions.host_query_reset = true;
                extensions.sampler_mirror_clamp_to_edge = true;
                extensions.spirv_1_4 = true;
                //extensions.buffer_device_address = true;
                extensions.descriptor_indexing = true;
                extensions.create_renderpass2 = true;
            }

            return extensions;
        }

        QueueFamilyIndices QueryQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            QueueFamilyIndices indices;

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }

                if (indices.isComplete()) {
                    break;
                }

                i++;
            }

            return indices;
        }
    }

    static struct
    {
        bool debugUtils = false;
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;

        VkSurfaceKHR surface = VK_NULL_HANDLE;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        uint32_t graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t copyQueueFamily = VK_QUEUE_FAMILY_IGNORED;

        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue computeQueue = VK_NULL_HANDLE;
        VkQueue copyQueue = VK_NULL_HANDLE;

        VkDevice device = VK_NULL_HANDLE;
        VmaAllocator allocator = VK_NULL_HANDLE;
        bool bufferDeviceAddress = false;
    } vk;

    extern VkResult CreateWindowSurface(VkInstance instance, Window* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

    class Vulkan_Texture final : public RefCounter<ITexture>
    {
    public:
        IDevice* device;
        TextureDesc desc;
        VkImage handle = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        Vulkan_Texture(IDevice* device_, void* nativeHandle, const TextureDesc& desc_, const TextureData* initialData)
            : device(device_)
            , desc(desc_)
        {
            if (desc.mipLevels == 0)
            {
                desc.mipLevels = (uint32_t)log2(std::max(desc_.width, desc_.height)) + 1;
            }

            if (nativeHandle != nullptr)
            {
                handle = (VkImage)nativeHandle;
            }
            else
            {
            }
        }

        ~Vulkan_Texture() override
        {

        }

        IDevice* GetDevice() const override { return device; }
        const TextureDesc& GetDesc() const override { return desc; }
        //uint64_t GetAllocatedSize() const override { return allocatedSize; }
        void ApiSetName(const std::string_view& newName) override
        {
        }
    };

    class Vulkan_Device final : public RefCounter<IDevice>
    {
    private:
        TextureHandle backBuffer;
        Format depthStencilFormat = Format::Undefined;
        TextureHandle depthStencilTexture;

    public:
        [[nodiscard]] static bool IsAvailable();

        Vulkan_Device(bool enableDebugLayers);
        ~Vulkan_Device() override;

        bool Initialize(_In_ Window* window, const PresentationParameters& presentationParameters);
        void WaitIdle() override;
        ICommandList* BeginFrame() override;
        void EndFrame() override;
        void Resize(uint32_t newWidth, uint32_t newHeight) override;

        ITexture* GetCurrentBackBuffer() const override { return backBuffer; }

        ITexture* GetBackBuffer(uint32_t index) const override
        {
            if (index == 0)
                return backBuffer;

            return nullptr;
        }

        uint32_t GetCurrentBackBufferIndex() const override { return 0; }
        uint32_t GetBackBufferCount() const override { return 1; }
        ITexture* GetBackBufferDepthStencilTexture() const override { return depthStencilTexture; }

        TextureHandle CreateTexture(const TextureDesc& desc, const TextureData* initialData = nullptr) override;
        TextureHandle CreateExternalTexture(void* nativeHandle, const TextureDesc& desc) override;
        ShaderHandle CreateShader(ShaderStages stage, const std::string& source, const std::string& entryPoint = "main") override;
        PipelineHandle CreateRenderPipeline(const RenderPipelineDesc& desc) override;
    };

    bool Vulkan_Device::IsAvailable()
    {
        static bool available_initialized = false;
        static bool available = false;

        if (available_initialized) {
            return available;
        }

        available_initialized = true;

        VkResult result = volkInitialize();
        if (result != VK_SUCCESS)
        {
            return false;
        }

        const uint32_t instanceVersion = volkGetInstanceVersion();
        if (instanceVersion < VK_API_VERSION_1_2)
        {
            return false;
        }

        available = true;
        return true;
    }

    Vulkan_Device::Vulkan_Device(bool enableDebugLayers)
    {
        // Create instance and debug utils first.
        uint32_t instanceExtensionCount;
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr));
        std::vector<VkExtensionProperties> availableInstanceExtensions(instanceExtensionCount);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, availableInstanceExtensions.data()));

        uint32_t instanceLayerCount;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
        std::vector<VkLayerProperties> availableInstanceLayers(instanceLayerCount);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers.data()));

        std::vector<const char*> instanceLayers;
        std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

        // Enable surface extensions depending on os
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
        instanceExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
        instanceExtensions.push_back(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
        instanceExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
        instanceExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
        instanceExtensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
#endif

        if (enableDebugLayers)
        {
            // Determine the optimal validation layers to enable that are necessary for useful debugging
            std::vector<const char*> optimalValidationLyers = GetOptimalValidationLayers(availableInstanceLayers);
            instanceLayers.insert(instanceLayers.end(), optimalValidationLyers.begin(), optimalValidationLyers.end());
        }

        // Check if VK_EXT_debug_utils is supported, which supersedes VK_EXT_Debug_Report
        for (auto& available_extension : availableInstanceExtensions)
        {
            if (strcmp(available_extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
            {
                vk.debugUtils = true;
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            else if (strcmp(available_extension.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
            {
                instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            }
        }

#if defined(_DEBUG) && defined(TODO)
        bool validationFeatures = false;
        if (validationMode == RHI::ValidationMode::GPU)
        {
            uint32_t layerInstanceExtensionCount;
            VK_CHECK(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerInstanceExtensionCount, nullptr));
            std::vector<VkExtensionProperties> availableLayerInstanceExtensions(layerInstanceExtensionCount);
            VK_CHECK(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerInstanceExtensionCount, availableLayerInstanceExtensions.data()));

            for (auto& availableExtension : availableLayerInstanceExtensions)
            {
                if (strcmp(availableExtension.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0)
                {
                    validationFeatures = true;
                    instanceExtensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
                }
            }
        }
#endif 

        VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
        //appInfo.pApplicationName = "Alimer";
        //appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        //appInfo.pEngineName = "Alimer";
        //appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
        appInfo.apiVersion = volkGetInstanceVersion();

        VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
        createInfo.ppEnabledLayerNames = instanceLayers.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        createInfo.ppEnabledExtensionNames = instanceExtensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

        if (enableDebugLayers && vk.debugUtils)
        {
            debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugUtilsCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;
            createInfo.pNext = &debugUtilsCreateInfo;
        }

#if defined(_DEBUG)&& defined(TODO)
        VkValidationFeaturesEXT validationFeaturesInfo = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
        if (validationFeatures)
        {
            static const VkValidationFeatureEnableEXT enable_features[2] = {
                VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
            };
            validationFeaturesInfo.enabledValidationFeatureCount = 2;
            validationFeaturesInfo.pEnabledValidationFeatures = enable_features;
            validationFeaturesInfo.pNext = createInfo.pNext;
            createInfo.pNext = &validationFeaturesInfo;
        }
#endif

        VkResult result = vkCreateInstance(&createInfo, nullptr, &vk.instance);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create Vulkan instance.");
            return;
        }

        volkLoadInstance(vk.instance);

        if (enableDebugLayers && vk.debugUtils)
        {
            result = vkCreateDebugUtilsMessengerEXT(vk.instance, &debugUtilsCreateInfo, nullptr, &vk.debugUtilsMessenger);
            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Could not create debug utils messenger");
            }
        }

        LOGI("Created VkInstance with version: {}.{}.{}",
            VK_VERSION_MAJOR(appInfo.apiVersion),
            VK_VERSION_MINOR(appInfo.apiVersion),
            VK_VERSION_PATCH(appInfo.apiVersion)
        );

        if (createInfo.enabledLayerCount)
        {
            LOGI("Enabled {} Validation Layers:", createInfo.enabledLayerCount);

            for (uint32_t i = 0; i < createInfo.enabledLayerCount; ++i)
            {
                LOGI("	\t{}", createInfo.ppEnabledLayerNames[i]);
            }
        }

        LOGI("Enabled {} Instance Extensions:", createInfo.enabledExtensionCount);
        for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
        {
            LOGI("	\t{}", createInfo.ppEnabledExtensionNames[i]);
        }
    }

    Vulkan_Device::~Vulkan_Device()
    {
        VK_CHECK(vkDeviceWaitIdle(vk.device));

        if (vk.allocator != VK_NULL_HANDLE)
        {
            VmaStats stats;
            vmaCalculateStats(vk.allocator, &stats);

            if (stats.total.usedBytes > 0)
            {
                LOGI("Total device memory leaked: {} bytes.", stats.total.usedBytes);
            }

            vmaDestroyAllocator(vk.allocator);
            vk.allocator = VK_NULL_HANDLE;
        }

        if (vk.device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(vk.device, nullptr);
            vk.device = VK_NULL_HANDLE;
        }

        if (vk.surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(vk.instance, vk.surface, nullptr);
            vk.surface = VK_NULL_HANDLE;
        }

        if (vk.debugUtilsMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(vk.instance, vk.debugUtilsMessenger, nullptr);
        }

        if (vk.instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(vk.instance, nullptr);
        }
    }

    bool Vulkan_Device::Initialize(_In_ Window* window, const PresentationParameters& presentationParameters)
    {
        VkResult result = CreateWindowSurface(vk.instance, window, nullptr, &vk.surface);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create window surface");
            return false;
        }

        // Enumerate physical devices and create logical device.
        VkPhysicalDeviceProperties2 properties2 = {};
        VkPhysicalDeviceVulkan11Properties properties_1_1 = {};
        VkPhysicalDeviceVulkan12Properties properties_1_2 = {};
        VkPhysicalDeviceAccelerationStructurePropertiesKHR acceleration_structure_properties = {};
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_properties = {};
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragment_shading_rate_properties = {};
        VkPhysicalDeviceMeshShaderPropertiesNV mesh_shader_properties = {};
        VkPhysicalDeviceDescriptorIndexingPropertiesEXT descriptor_indexing_properties = {};

        VkPhysicalDeviceFeatures2 features2 = {};
        VkPhysicalDeviceVulkan11Features features_1_1 = {};
        VkPhysicalDeviceVulkan12Features features_1_2 = {};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features = {};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_features = {};
        VkPhysicalDeviceRayQueryFeaturesKHR raytracing_query_features = {};
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragment_shading_rate_features = {};
        VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = {};
        VkPhysicalDevicePerformanceQueryFeaturesKHR perf_counter_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR };
        VkPhysicalDeviceHostQueryResetFeatures host_query_reset_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES };
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing_features = {};

        // Enumerate physical devices and create logical device.
        {
            uint32_t deviceCount = 0;
            VK_CHECK(vkEnumeratePhysicalDevices(vk.instance, &deviceCount, nullptr));

            if (deviceCount == 0)
            {
                LOGF("Vulkan: Failed to find GPUs with Vulkan support");
                //ErrorDialog("Error", "Vulkan: Failed to find GPUs with Vulkan support");
                return false;
            }

            LOGD("Vulkan: Found {} GPU's", deviceCount);

            std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
            VK_CHECK(vkEnumeratePhysicalDevices(vk.instance, &deviceCount, physicalDevices.data()));

            std::vector<const char*> enabledExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME,
            };

            for (const VkPhysicalDevice& physicalDevice : physicalDevices)
            {
                bool suitable = true;

                uint32_t extensionCount;
                VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr));
                std::vector<VkExtensionProperties> availableExtensions(extensionCount);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data()));

                PhysicalDeviceExtensions physicalDeviceExt = QueryPhysicalDeviceExtensions(physicalDevice);
                suitable = physicalDeviceExt.swapchain && physicalDeviceExt.depth_clip_enable;
                if (!suitable)
                {
                    continue;
                }

                QueueFamilyIndices familyIndices = QueryQueueFamilies(physicalDevice, vk.surface);
                if (!familyIndices.isComplete())
                {
                    continue;
                }

                features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                features_1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                features_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                features2.pNext = &features_1_1;
                features_1_1.pNext = &features_1_2;
                void** features_chain = &features_1_2.pNext;
                acceleration_structure_features = {};
                raytracing_features = {};
                raytracing_query_features = {};
                fragment_shading_rate_features = {};
                mesh_shader_features = {};

                properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                properties_1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
                properties_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
                properties2.pNext = &properties_1_1;
                properties_1_1.pNext = &properties_1_2;
                void** properties_chain = &properties_1_2.pNext;
                acceleration_structure_properties = {};
                raytracing_properties = {};
                fragment_shading_rate_properties = {};
                mesh_shader_properties = {};

                if (physicalDeviceExt.memory_budget)
                {
                    enabledExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
                }

                // For performance queries, we also use host query reset since queryPool resets cannot live in the same command buffer as beginQuery
                if (physicalDeviceExt.performance_query &&
                    physicalDeviceExt.host_query_reset)
                {
                    VkPhysicalDeviceFeatures2 physical_device_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
                    physical_device_features.pNext = &perf_counter_features;
                    vkGetPhysicalDeviceFeatures2(physicalDevice, &physical_device_features);

                    physical_device_features.pNext = &host_query_reset_features;
                    vkGetPhysicalDeviceFeatures2(physicalDevice, &physical_device_features);

                    if (perf_counter_features.performanceCounterQueryPools && host_query_reset_features.hostQueryReset)
                    {
                        *features_chain = &perf_counter_features;
                        features_chain = &perf_counter_features.pNext;
                        *features_chain = &host_query_reset_features;
                        features_chain = &host_query_reset_features.pNext;

                        enabledExtensions.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
                        enabledExtensions.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
                    }
                }

                if (physicalDeviceExt.spirv_1_4)
                {
                    // Required for VK_KHR_ray_tracing_pipeline
                    enabledExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);

                    // Required by VK_KHR_spirv_1_4
                    enabledExtensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
                }

                if (physicalDeviceExt.buffer_device_address)
                {
                    // Required by VK_KHR_acceleration_structure
                    enabledExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
                }

                //if (physicalDeviceExt.descriptor_indexing)
                //{
                //    // Required by VK_KHR_acceleration_structure
                //    enabledExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
                //
                //    descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
                //    *features_chain = &descriptor_indexing_features;
                //    features_chain = &descriptor_indexing_features.pNext;
                //}

                if (physicalDeviceExt.accelerationStructure)
                {
                    ALIMER_ASSERT(physicalDeviceExt.buffer_device_address);
                    ALIMER_ASSERT(physicalDeviceExt.descriptor_indexing);
                    ALIMER_ASSERT(physicalDeviceExt.deferred_host_operations);

                    // Required by VK_KHR_acceleration_structure
                    enabledExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

                    enabledExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
                    acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
                    *features_chain = &acceleration_structure_features;
                    features_chain = &acceleration_structure_features.pNext;
                    acceleration_structure_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
                    *properties_chain = &acceleration_structure_properties;
                    properties_chain = &acceleration_structure_properties.pNext;

                    if (physicalDeviceExt.raytracingPipeline)
                    {
                        enabledExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
                        raytracing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
                        *features_chain = &raytracing_features;
                        features_chain = &raytracing_features.pNext;
                        raytracing_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
                        *properties_chain = &raytracing_properties;
                        properties_chain = &raytracing_properties.pNext;
                    }

                    if (physicalDeviceExt.rayQuery)
                    {
                        enabledExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
                        raytracing_query_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
                        *features_chain = &raytracing_query_features;
                        features_chain = &raytracing_query_features.pNext;
                    }
                }

                if (physicalDeviceExt.create_renderpass2)
                {
                    enabledExtensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
                }

                if (physicalDeviceExt.fragment_shading_rate)
                {
                    ALIMER_ASSERT(physicalDeviceExt.create_renderpass2);

                    enabledExtensions.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
                    fragment_shading_rate_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
                    *features_chain = &fragment_shading_rate_features;
                    features_chain = &fragment_shading_rate_features.pNext;
                    fragment_shading_rate_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
                    *properties_chain = &fragment_shading_rate_properties;
                    properties_chain = &fragment_shading_rate_properties.pNext;
                }

                if (physicalDeviceExt.NV_mesh_shader)
                {
                    enabledExtensions.push_back(VK_NV_MESH_SHADER_EXTENSION_NAME);
                    mesh_shader_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
                    *features_chain = &mesh_shader_features;
                    features_chain = &mesh_shader_features.pNext;
                    mesh_shader_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV;
                    *properties_chain = &mesh_shader_properties;
                    properties_chain = &mesh_shader_properties.pNext;
                }

                vkGetPhysicalDeviceProperties2(physicalDevice, &properties2);

                bool discrete = properties2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                if (discrete || vk.physicalDevice == VK_NULL_HANDLE)
                {
                    vk.physicalDevice = physicalDevice;
                    if (discrete)
                    {
                        break; // if this is discrete GPU, look no further (prioritize discrete GPU)
                    }
                }
            }

            if (vk.physicalDevice == VK_NULL_HANDLE)
            {
                LOGE("Vulkan: Failed to find a suitable GPU");
                return false;
            }

            // TODO: Init caps

            // Find queue families:
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(vk.physicalDevice, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(vk.physicalDevice, &queueFamilyCount, queueFamilies.data());

            std::vector<uint32_t> queueOffsets(queueFamilyCount);
            std::vector<std::vector<float>> queuePriorities(queueFamilyCount);

            uint32_t graphicsQueueIndex = 0;
            uint32_t computeQueueIndex = 0;
            uint32_t copyQueueIndex = 0;

            const auto FindVacantQueue = [&](uint32_t& family, uint32_t& index,
                VkQueueFlags required, VkQueueFlags ignore_flags,
                float priority) -> bool
            {
                for (uint32_t familyIndex = 0; familyIndex < queueFamilyCount; familyIndex++)
                {
                    if ((queueFamilies[familyIndex].queueFlags & ignore_flags) != 0)
                        continue;

                    // A graphics queue candidate must support present for us to select it.
                    if ((required & VK_QUEUE_GRAPHICS_BIT) != 0)
                    {
                        VkBool32 supported = VK_FALSE;
                        if (vkGetPhysicalDeviceSurfaceSupportKHR(vk.physicalDevice, familyIndex, vk.surface, &supported) != VK_SUCCESS || !supported)
                            continue;
                    }

                    if (queueFamilies[familyIndex].queueCount &&
                        (queueFamilies[familyIndex].queueFlags & required) == required)
                    {
                        family = familyIndex;
                        queueFamilies[familyIndex].queueCount--;
                        index = queueOffsets[familyIndex]++;
                        queuePriorities[familyIndex].push_back(priority);
                        return true;
                    }
                }

                return false;
            };

            if (!FindVacantQueue(vk.graphicsQueueFamily, graphicsQueueIndex,
                VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 0.5f))
            {
                LOGE("Vulkan: Could not find suitable graphics queue.");
                return false;
            }

            // Prefer another graphics queue since we can do async graphics that way.
            // The compute queue is to be treated as high priority since we also do async graphics on it.
            if (!FindVacantQueue(vk.computeQueueFamily, computeQueueIndex, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 1.0f) &&
                !FindVacantQueue(vk.computeQueueFamily, computeQueueIndex, VK_QUEUE_COMPUTE_BIT, 0, 1.0f))
            {
                // Fallback to the graphics queue if we must.
                vk.computeQueueFamily = vk.graphicsQueueFamily;
                computeQueueIndex = graphicsQueueIndex;
            }

            // For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
            // If not, fallback to a dedicated compute queue.
            // Finally, fallback to same queue as compute.
            if (!FindVacantQueue(vk.copyQueueFamily, copyQueueIndex, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0.5f) &&
                !FindVacantQueue(vk.copyQueueFamily, copyQueueIndex, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, 0.5f))
            {
                vk.copyQueueFamily = vk.computeQueueFamily;
                copyQueueIndex = computeQueueIndex;
            }

            VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

            std::vector<VkDeviceQueueCreateInfo> queueInfos;
            for (uint32_t familyIndex = 0; familyIndex < queueFamilyCount; familyIndex++)
            {
                if (queueOffsets[familyIndex] == 0)
                    continue;

                VkDeviceQueueCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
                info.queueFamilyIndex = familyIndex;
                info.queueCount = queueOffsets[familyIndex];
                info.pQueuePriorities = queuePriorities[familyIndex].data();
                queueInfos.push_back(info);
            }

            createInfo.pNext = &features2;
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
            createInfo.pQueueCreateInfos = queueInfos.data();
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
            createInfo.pEnabledFeatures = nullptr;
            createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
            createInfo.ppEnabledExtensionNames = enabledExtensions.data();

            VkResult result = vkCreateDevice(vk.physicalDevice, &createInfo, nullptr, &vk.device);
            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Cannot create device");
                return false;
            }

            volkLoadDevice(vk.device);

            // Queues
            //queues[(uint32_t)CommandQueueType::Graphics].queueFamilyIndex = vk.graphicsQueueFamily;
            vkGetDeviceQueue(vk.device, vk.graphicsQueueFamily, graphicsQueueIndex, &vk.graphicsQueue);

            // Compute queue
            //queues[(uint32_t)CommandQueueType::Graphics].queueFamilyIndex = computeQueueFamily;
            vkGetDeviceQueue(vk.device, vk.computeQueueFamily, computeQueueIndex, &vk.computeQueue);

            // Copy
            vkGetDeviceQueue(vk.device, vk.copyQueueFamily, copyQueueIndex, &vk.copyQueue);

            LOGI("Vendor : {}", GetVendorName(properties2.properties.vendorID));
            LOGI("Name   : {}", properties2.properties.deviceName);
            LOGI("Type   : {}", kDeviceTypes[properties2.properties.deviceType]);
            LOGI("API    : {}.{}.{}",
                VK_VERSION_MAJOR(properties2.properties.apiVersion),
                VK_VERSION_MINOR(properties2.properties.apiVersion),
                VK_VERSION_PATCH(properties2.properties.apiVersion)
            );
            LOGI("Driver : {}.{}.{}",
                VK_VERSION_MAJOR(properties2.properties.driverVersion),
                VK_VERSION_MINOR(properties2.properties.driverVersion),
                VK_VERSION_PATCH(properties2.properties.driverVersion)
            );
            LOGI("Enabled {} Device Extensions:", createInfo.enabledExtensionCount);
            for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
            {
                LOGI("	\t{}", createInfo.ppEnabledExtensionNames[i]);
            }

#ifdef _DEBUG
            LOGD("Graphics queue: family {}, index {}.", vk.graphicsQueueFamily, graphicsQueueIndex);
            LOGD("Compute queue: family {}, index {}.", vk.computeQueueFamily, computeQueueIndex);
            LOGD("Transfer queue: family {}, index {}.", vk.copyQueueFamily, copyQueueIndex);
#endif
        }

        vk.bufferDeviceAddress = features_1_2.bufferDeviceAddress;

        // Initialize Vulkan Memory Allocator helper
        {
            VmaAllocatorCreateInfo allocatorInfo = {};
            allocatorInfo.physicalDevice = vk.physicalDevice;
            allocatorInfo.device = vk.device;
            allocatorInfo.instance = vk.instance;
            if (features_1_2.bufferDeviceAddress)
            {
                allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
            }

            VK_CHECK(vmaCreateAllocator(&allocatorInfo, &vk.allocator));
        }

        return true;
    }

    void Vulkan_Device::WaitIdle()
    {
        VK_CHECK(vkDeviceWaitIdle(vk.device));
    }

    ICommandList* Vulkan_Device::BeginFrame()
    {
        return nullptr;
    }

    void Vulkan_Device::EndFrame()
    {
    }

    void Vulkan_Device::Resize(uint32_t newWidth, uint32_t newHeight)
    {

    }

    TextureHandle Vulkan_Device::CreateTexture(const TextureDesc& desc, const TextureData* initialData)
    {
        auto result = new Vulkan_Texture(this, nullptr, desc, initialData);

        if (result->handle != VK_NULL_HANDLE)
            return TextureHandle::Create(result);

        delete result;
        return nullptr;
    }

    TextureHandle Vulkan_Device::CreateExternalTexture(void* nativeHandle, const TextureDesc& desc)
    {
        auto result = new Vulkan_Texture(this, nativeHandle, desc, nullptr);

        if (result->handle != VK_NULL_HANDLE)
            return TextureHandle::Create(result);

        delete result;
        return nullptr;
    }

    ShaderHandle Vulkan_Device::CreateShader(ShaderStages stage, const std::string& source, const std::string& entryPoint)
    {
        return nullptr;
    }

    PipelineHandle Vulkan_Device::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        return nullptr;
    }

    DeviceHandle CreateVulkanDevice(alimer::Window* window, const PresentationParameters& presentationParameters)
    {
        if (!Vulkan_Device::IsAvailable())
            return nullptr;

        auto device = new Vulkan_Device(presentationParameters.validationMode != ValidationMode::Disabled);
        if (device->Initialize(window, presentationParameters))
        {
            return DeviceHandle::Create(device);
        }

        delete device;
        return nullptr;
    }
}

#endif /* defined(ALIMER_RHI_VULKAN) */
