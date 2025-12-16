# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")

# ShaderMakeBlob
add_subdirectory(ShaderMakeBlob)

# fmt
add_subdirectory(fmt EXCLUDE_FROM_ALL)
set_property(TARGET fmt PROPERTY FOLDER "ThirdParty")
#install (DIRECTORY spdlog/include/spdlog DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/ThirdParty)

# spdlog
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "Use external fmt library instead of bundled" FORCE)
set(SPDLOG_USE_STD_FORMAT OFF CACHE BOOL "Use external fmt library instead of bundled" FORCE)
add_subdirectory(spdlog EXCLUDE_FROM_ALL)
set_property(TARGET spdlog PROPERTY FOLDER "ThirdParty")
install (DIRECTORY spdlog/include/spdlog DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/ThirdParty)

# unordered_dense
add_subdirectory(unordered_dense)
set_property(TARGET unordered_dense PROPERTY FOLDER "ThirdParty")
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/unordered_dense/include/ankerl
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/ThirdParty
)

# lz4
add_subdirectory(lz4)
set_property(TARGET lz4 PROPERTY FOLDER "ThirdParty")

# RHI
if (ALIMER_RHI_VULKAN)
    # vulkan
    add_library(vulkan INTERFACE)
    set(VULKAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include)
    target_sources(vulkan INTERFACE 
        ${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h
    )
    target_include_directories(vulkan SYSTEM INTERFACE ${VULKAN_INCLUDE_DIR})

    # vma
    set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vma/include)
    add_library(vma INTERFACE)
    target_sources(vma INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
    target_include_directories(vma SYSTEM INTERFACE ${VMA_DIR})
    target_link_libraries(vma INTERFACE vulkan)

    # spirv_reflect
    set(SPIRV_REFLECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect")
    add_library(spirv_reflect STATIC 
        "${SPIRV_REFLECT_DIR}/spirv_reflect.c"
        "${SPIRV_REFLECT_DIR}/spirv_reflect.h"
    )
    set_target_properties(spirv_reflect PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_include_directories(spirv_reflect SYSTEM PUBLIC ${SPIRV_REFLECT_DIR})
    set_target_properties(spirv_reflect PROPERTIES FOLDER "ThirdParty")
endif (ALIMER_RHI_VULKAN)

if (ALIMER_RHI_D3D12 OR ALIMER_SHADER_COMPILER)
    add_library(DirectX-Headers INTERFACE)
    set(DIRECTX_HEADERS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DirectX-Headers/include)
    target_sources(DirectX-Headers INTERFACE 
        ${DIRECTX_HEADERS_INCLUDE_DIR}/directx/d3d12.h
        ${DIRECTX_HEADERS_INCLUDE_DIR}/directx/d3dx12.h
        ${DIRECTX_HEADERS_INCLUDE_DIR}/directx/d3d12shader.h
    )
    target_include_directories(DirectX-Headers INTERFACE ${DIRECTX_HEADERS_INCLUDE_DIR})

    # Only D3D12 RHI
    if (ALIMER_RHI_D3D12)
        set(D3D12MA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/d3d12ma)
        add_library (d3d12ma STATIC
            ${D3D12MA_DIR}/include/D3D12MemAlloc.h
            ${D3D12MA_DIR}/src/D3D12MemAlloc.cpp
        )

        target_include_directories(d3d12ma
            INTERFACE $<BUILD_INTERFACE:${D3D12MA_DIR}/include>
            PRIVATE   $<BUILD_INTERFACE:${D3D12MA_DIR}/include>
        )

        target_compile_definitions(d3d12ma PRIVATE D3D12MA_USING_DIRECTX_HEADERS)
        target_link_libraries(d3d12ma PUBLIC DirectX-Headers)
        set_property(TARGET d3d12ma PROPERTY FOLDER "ThirdParty")
    endif ()
endif (ALIMER_RHI_D3D12 OR ALIMER_SHADER_COMPILER)

# Fetch & setup Metal C++ headers
if(ALIMER_RHI_METAL)
    message(STATUS "Fetching Metal C++ headers ...")
    add_library(alimer-rhi-metal-cpp INTERFACE)
    target_include_directories(alimer-rhi-metal-cpp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp")
    target_link_libraries(alimer-rhi-metal-cpp INTERFACE "-framework Metal")
endif (ALIMER_RHI_METAL)

# Jolt Physics
set(OBJECT_LAYER_BITS 32)
set(ENABLE_OBJECT_STREAM OFF)
add_subdirectory (JoltPhysics)
set_property(TARGET Jolt PROPERTY FOLDER "ThirdParty")
