// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_RHI_VULKAN)
#include "Graphics/Graphics.h"
#include "Graphics/Texture.h"
#include "Window.h"
#include "Core/Log.h"
#include "PlatformInclude.h"
#include "volk.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "spirv_reflect.h"

/// Helper macro to test the result of Vulkan calls which can return an error.
#define VK_CHECK(x) \
	do \
	{ \
		VkResult err = x; \
		if (err) \
		{ \
			LOGE("Detected Vulkan error: {}", alimer::ToString(err)); \
		} \
	} while (0)

#define VK_LOG_ERROR(result, message) LOGE("Vulkan: {}, error: {}", message, alimer::ToString(result));

namespace alimer
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
    }

    static struct
    {
        bool debugUtils = false;
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
    } vk;

    class Vulkan_Texture final : public RefCounter<Texture>
    {
    public:
        Vulkan_Texture(u32 width, u32 height)
        {

        }

        ~Vulkan_Texture() override
        {

        }
    };

    class Vulkan_Graphics final : public Graphics
    {
    private:

    public:
        [[nodiscard]] static bool IsAvailable();

        Vulkan_Graphics(bool enableDebugLayers);
        ~Vulkan_Graphics() override;

        bool Initialize(_In_ Window* window, const PresentationParameters& presentationParameters) override;
        bool BeginFrame() override;
        void EndFrame() override;
        void Resize(u32 newWidth, u32 newHeight) override;

        RefCountPtr<Texture> CreateTexture(u32 width, u32 height) override;
    };

    bool Vulkan_Graphics::IsAvailable()
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

    Vulkan_Graphics::Vulkan_Graphics(bool enableDebugLayers)
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

    Vulkan_Graphics::~Vulkan_Graphics()
    {
        if (vk.debugUtilsMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(vk.instance, vk.debugUtilsMessenger, nullptr);
        }

        if (vk.instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(vk.instance, nullptr);
        }
    }

    bool Vulkan_Graphics::Initialize(_In_ Window* window, const PresentationParameters& presentationParameters)
    {
        return true;
    }

    bool Vulkan_Graphics::BeginFrame()
    {
        return true;
    }

    void Vulkan_Graphics::EndFrame()
    {
    }

    void Vulkan_Graphics::Resize(u32 newWidth, u32 newHeight)
    {

    }

    RefCountPtr<Texture> Vulkan_Graphics::CreateTexture(u32 width, u32 height)
    {
        auto result = new Vulkan_Texture(width, height);

        //if (result->handle)
        return TextureRef::Create(result);

        //delete result;
        //return nullptr;
    }

    bool Vulkan_IsAvailable()
    {
        return Vulkan_Graphics::IsAvailable();
    }

    bool Vulkan_Initialize(Window* window, const PresentationParameters& presentationParameters)
    {
        if (!Vulkan_Graphics::IsAvailable())
            return false;

        gGraphics().Start<Vulkan_Graphics>(presentationParameters.validationMode != ValidationMode::Disabled);
        return gGraphics().Initialize(window, presentationParameters);
    }
}

#endif /* defined(ALIMER_RHI_VULKAN) */
