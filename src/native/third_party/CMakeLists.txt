# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")

if (ALIMER_USE_SDL AND NOT (EMSCRIPTEN OR ANDROID OR WINDOWS_STORE))
    CPMAddPackage(
        NAME SDL
        GITHUB_REPOSITORY libsdl-org/SDL
        GIT_TAG release-3.2.26
        OPTIONS
            "SDL_SHARED OFF"
            "SDL_STATIC ON"
            "SDL_LIBC ON"
            "SDL_TIMERS ON"
            "SDL_TEST_LIBRARY OFF"
            "SDL_DISABLE_INSTALL ON"
            "SDL_DISABLE_INSTALL_DOCS ON"
            # Disable everything we don't care about.
            "SDL_ATOMIC OFF"
            "SDL_AUDIO OFF"
            "SDL_RENDER OFF"
            "SDL_HAPTIC OFF"
            "SDL_POWER OFF"
            "SDL_FILE OFF"
            "SDL_CPUINFO OFF"
            "SDL_FILESYSTEM OFF"
            "SDL_SENSOR OFF"
            "SDL_LOCALE OFF"
            "SDL_MISC OFF"
            "SDL_CAMERA OFF"
            "SDL_DIALOG OFF"
            "SDL_GPU OFF"
        EXCLUDE_FROM_ALL YES
    )

    set_property(TARGET "SDL3-static" PROPERTY FOLDER "ThirdParty")
    set_property(TARGET "SDL_uclibc" PROPERTY FOLDER "ThirdParty")
endif ()

if (ALIMER_GPU_VULKAN)
    # vulkan
    add_library(vulkan INTERFACE)
    set(VULKAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include)
    target_sources(vulkan INTERFACE 
        ${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h
    )
    target_include_directories(vulkan INTERFACE ${VULKAN_INCLUDE_DIR})

    # spirv_reflect
    set(SPIRV_REFLECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect")
    add_library(spirv_reflect STATIC 
        "${SPIRV_REFLECT_DIR}/spirv_reflect.c"
        "${SPIRV_REFLECT_DIR}/spirv_reflect.h"
    )
    set_target_properties(spirv_reflect PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_include_directories(spirv_reflect SYSTEM PUBLIC ${SPIRV_REFLECT_DIR})
    set_target_properties(spirv_reflect PROPERTIES FOLDER "ThirdParty")
endif()

if (ALIMER_GPU_D3D12)
    add_library(DirectX-Headers INTERFACE)
    set(DIRECTX_HEADERS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DirectX-Headers/include)
    target_sources(DirectX-Headers INTERFACE 
        ${DIRECTX_HEADERS_INCLUDE_DIR}/directx/d3d12.h
        ${DIRECTX_HEADERS_INCLUDE_DIR}/directx/d3dx12.h
        ${DIRECTX_HEADERS_INCLUDE_DIR}/directx/d3d12shader.h
    )
    target_include_directories(DirectX-Headers INTERFACE ${DIRECTX_HEADERS_INCLUDE_DIR})

    # d3d12ma
    set(D3D12_DIR "${CMAKE_CURRENT_SOURCE_DIR}/D3D12MemoryAllocator")
    add_library(d3d12ma STATIC 
        "${D3D12_DIR}/D3D12MemAlloc.cpp"
        "${D3D12_DIR}/D3D12MemAlloc.h"
    )
    target_include_directories(d3d12ma PUBLIC ${D3D12_DIR})
    target_link_libraries(d3d12ma PUBLIC DirectX-Headers)
    target_compile_definitions(d3d12ma PUBLIC D3D12MA_USING_DIRECTX_HEADERS)
    set_target_properties(d3d12ma PROPERTIES FOLDER "ThirdParty")
endif ()

if(ALIMER_PHYSICS)
    CPMAddPackage(
        NAME JoltPhysics
        GITHUB_REPOSITORY jrouwe/JoltPhysics
        GIT_TAG v5.4.0
        SOURCE_SUBDIR Build
        OPTIONS
            "OBJECT_LAYER_BITS 32"
            "DOUBLE_PRECISION OFF"
            "GENERATE_DEBUG_SYMBOLS OFF"
            "CROSS_PLATFORM_DETERMINISTIC OFF"
            "ENABLE_INSTALL OFF"
            "ENABLE_ALL_WARNINGS OFF"
            "CPP_EXCEPTIONS_ENABLED OFF"
            "CPP_RTTI_ENABLED OFF"
            "USE_STATIC_MSVC_RUNTIME_LIBRARY OFF"
            "ENABLE_OBJECT_STREAM OFF"
    )
    set_property(TARGET Jolt PROPERTY FOLDER "ThirdParty")
endif()
