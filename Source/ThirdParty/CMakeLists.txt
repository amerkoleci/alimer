# GLFW
if(NOT (EMSCRIPTEN OR ANDROID))
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
  set(GLFW_INSTALL OFF CACHE BOOL "")
    
  add_subdirectory(glfw EXCLUDE_FROM_ALL)
  set_property(TARGET glfw PROPERTY FOLDER "ThirdParty")
endif()

# spdlog
add_subdirectory(spdlog EXCLUDE_FROM_ALL)
set_property(TARGET spdlog PROPERTY FOLDER "ThirdParty")

if(ALIMER_RHI_VULKAN)
    # vulkan
    add_library(vulkan INTERFACE)
    set(VULKAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include)
    target_sources(vulkan INTERFACE 
        ${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h
        ${VULKAN_INCLUDE_DIR}/vulkan/vk_platform.h
        ${VULKAN_INCLUDE_DIR}/vulkan/vulkan_core.h
    )
    target_include_directories(vulkan INTERFACE ${VULKAN_INCLUDE_DIR})
    target_compile_definitions(vulkan INTERFACE VK_NO_PROTOTYPES)
    #target_compile_definitions(vulkan INTERFACE VK_ENABLE_BETA_EXTENSIONS)

    if(ANDROID)
        target_sources(vulkan INTERFACE ${VULKAN_INCLUDE_DIR}/vulkan/vulkan_android.h)
        target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
    elseif(WIN32)
        target_sources(vulkan INTERFACE ${VULKAN_INCLUDE_DIR}/vulkan/vulkan_win32.h)
        target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WIN32_KHR)
    elseif(APPLE)
        target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_MACOS_MVK)
    elseif(UNIX)
        # See whether X11 is available. If not, fall back to direct-to-display mode.
        find_package(X11 QUIET)
        if (X11_FOUND)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XCB_KHR)
        else()
            set(DIRECT_TO_DISPLAY TRUE)
            set(DIRECT_TO_DISPLAY TRUE PARENT_SCOPE)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_DISPLAY_KHR)
        endif()
    endif()

    # volk
    set(VOLK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/volk")
    set(VOLK_FILES
        "${VOLK_DIR}/volk.c"
        "${VOLK_DIR}/volk.h")

    add_library(volk STATIC ${VOLK_FILES})

    target_link_libraries(volk PUBLIC vulkan)
    target_include_directories(volk PUBLIC ${VOLK_DIR})
    set_property(TARGET volk PROPERTY FOLDER "ThirdParty")

    # vma
    add_library(vma INTERFACE)
    set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vma/include)
    target_sources(vma INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
    target_include_directories(vma INTERFACE ${VMA_DIR})
    target_link_libraries(vma INTERFACE vulkan)

    # spirv-reflect
    set(SPIRV_REFLECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/spirv-reflect")
    set(SPIRV_REFLECT_FILES
        "${SPIRV_REFLECT_DIR}/spirv_reflect.c"
        "${SPIRV_REFLECT_DIR}/spirv_reflect.h"
    )
 
    add_library(spirv-reflect STATIC ${SPIRV_REFLECT_FILES})
    target_include_directories(spirv-reflect PUBLIC ${SPIRV_REFLECT_DIR})
    set_property(TARGET spirv-reflect PROPERTY FOLDER "ThirdParty")
endif()