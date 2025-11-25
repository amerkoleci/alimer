# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")

# ktx
if (ALIMER_IMAGE_KTX)
    # libktx
    set(KTX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ktx)

    set(KTX_SOURCES
        ${KTX_DIR}/external/dfdutils/createdfd.c
        ${KTX_DIR}/external/dfdutils/colourspaces.c
        ${KTX_DIR}/external/dfdutils/dfd.h
        ${KTX_DIR}/external/dfdutils/dfd2vk.inl
        ${KTX_DIR}/external/dfdutils/interpretdfd.c
        ${KTX_DIR}/external/dfdutils/printdfd.c
        ${KTX_DIR}/external/dfdutils/queries.c
        ${KTX_DIR}/external/dfdutils/vk2dfd.c
        ${KTX_DIR}/external/etcdec/etcdec.cxx
        ${KTX_DIR}/lib/src/checkheader.c
        ${KTX_DIR}/lib/src/etcunpack.cxx
        ${KTX_DIR}/lib/src/filestream.c
        ${KTX_DIR}/lib/src/filestream.h
        ${KTX_DIR}/lib/src/formatsize.h
        ${KTX_DIR}/lib/src/hashlist.c
        ${KTX_DIR}/lib/src/info.c
        ${KTX_DIR}/lib/src/ktxint.h
        ${KTX_DIR}/lib/src/memstream.c
        ${KTX_DIR}/lib/src/memstream.h
        ${KTX_DIR}/lib/src/strings.c
        ${KTX_DIR}/lib/src/swap.c
        ${KTX_DIR}/lib/src/uthash.h
        ${KTX_DIR}/lib/src/texture.c
        ${KTX_DIR}/lib/src/texture.h
        ${KTX_DIR}/utils/unused.h
    
        # Basis Universal
        ${KTX_DIR}/lib/src/basis_sgd.h
        ${KTX_DIR}/lib/src/basis_transcode.cpp
        ${KTX_DIR}/lib/src/miniz_wrapper.cpp
        ${KTX_DIR}/external/basis_universal/encoder/basisu_miniz.h
        ${KTX_DIR}/external/basis_universal/transcoder/basisu_containers.h
        ${KTX_DIR}/external/basis_universal/transcoder/basisu_containers_impl.h
        ${KTX_DIR}/external/basis_universal/transcoder/basisu_file_headers.h
        ${KTX_DIR}/external/basis_universal/transcoder/basisu_transcoder_internal.h
        ${KTX_DIR}/external/basis_universal/transcoder/basisu_transcoder_uastc.h
        ${KTX_DIR}/external/basis_universal/transcoder/basisu_transcoder.cpp
        ${KTX_DIR}/external/basis_universal/transcoder/basisu_transcoder.h
        ${KTX_DIR}/external/basis_universal/transcoder/basisu.h
        ${KTX_DIR}/external/basis_universal/zstd/zstd.c

        # KT1
        ${KTX_DIR}/lib/src/texture1.c
        ${KTX_DIR}/lib/src/texture1.h

        # KTX2
        ${KTX_DIR}/lib/src/texture2.c
        ${KTX_DIR}/lib/src/texture2.h

        # GL support
        ${KTX_DIR}/lib/src/gl_format.h

        # Vulkan support
        ${KTX_DIR}/lib/src/vk_format.h
        ${KTX_DIR}/lib/src/vkformat_check.c
        ${KTX_DIR}/lib/src/vkformat_enum.h
        #${KTX_DIR}/lib/vkformat_str.c
        ${KTX_DIR}/lib/src/vkformat_typesize.c
        ${KTX_DIR}/lib/src/vkformat_check_variant.c
        #${KTX_DIR}/lib/vk_funcs.c
        #${KTX_DIR}/lib/vk_funcs.h
        #${KTX_DIR}/lib/vkloader.c
    )

    set(KTX_INCLUDE_DIRS
        ${KTX_DIR}/lib/include
        ${KTX_DIR}/lib/src
        ${KTX_DIR}/utils
        ${KTX_DIR}/external
        ${KTX_DIR}/external/basis_universal
        ${KTX_DIR}/external/basis_universal/zstd
        ${KTX_DIR}/external/basis_universal/transcoder
        ${KTX_DIR}/other_include
    )

    add_library(ktx STATIC ${KTX_SOURCES})

    target_compile_definitions(ktx PUBLIC LIBKTX)
    if (WIN32)
        target_compile_definitions(ktx PUBLIC "KTX_API=__declspec(dllexport)")
    endif()
    target_compile_definitions(ktx PUBLIC KTX_FEATURE_WRITE=0)
    target_compile_definitions(ktx PUBLIC BASISD_SUPPORT_KTX2_ZSTD=0)
    target_compile_definitions(ktx PUBLIC BASISU_NO_ITERATOR_DEBUG_LEVEL)

    target_include_directories(ktx SYSTEM PUBLIC ${KTX_INCLUDE_DIRS})

    set_target_properties(ktx PROPERTIES FOLDER "ThirdParty" POSITION_INDEPENDENT_CODE ON)
endif()

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
