# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")

# CPM
#CPMAddPackage(
#    NAME ktx
#    GITHUB_REPOSITORY KhronosGroup/KTX-Software
#    GIT_TAG main
#    OPTIONS
#        "BUILD_SHARED_LIBS ON"
#        "KTX_FEATURE_JS OFF"
#        "LIBKTX_VERSION_READ_ONLY ON"
#        "LIBKTX_VERSION_FULL OFF"
#        "LIBKTX_FEATURE_VK_UPLOAD OFF"
#        "LIBKTX_FEATURE_GL_UPLOAD OFF"
#)
#set_property(TARGET ktx PROPERTY FOLDER "ThirdParty")

# ktx
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
    #${KTX_DIR}/lib/src/info.c
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
    ${KTX_DIR}/external/dfdutils
    ${KTX_DIR}/other_include
)

add_library(ktx ${ALIMER_LIBRARY_TYPE} ${KTX_SOURCES})

target_compile_definitions(ktx PUBLIC LIBKTX)
if (WIN32)
    target_compile_definitions(ktx PUBLIC "KTX_API=__declspec(dllexport)")
else()
    target_compile_definitions(ktx PUBLIC "KTX_API=__attribute__((visibility(\"default\")))")
endif()
target_compile_definitions(ktx PUBLIC KTX_FEATURE_WRITE=0)
target_compile_definitions(ktx PUBLIC BASISD_SUPPORT_KTX2_ZSTD=0)
target_compile_definitions(ktx PUBLIC BASISU_NO_ITERATOR_DEBUG_LEVEL)

target_include_directories(ktx SYSTEM PUBLIC ${KTX_INCLUDE_DIRS})

set_target_properties(ktx PROPERTIES
    FOLDER "ThirdParty"
    POSITION_INDEPENDENT_CODE ON
)

# Install
if (ALIMER_INSTALL)
    install (
        DIRECTORY include
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ktx
        FILES_MATCHING PATTERN *.h
    )

    install(TARGETS ktx
        ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION  ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
    )
endif()

# vulkan
CPMAddPackage(
    NAME Vulkan-Headers
    GITHUB_REPOSITORY "KhronosGroup/Vulkan-Headers"
    GIT_TAG "v1.4.365"
    OPTIONS
        "VULKAN_HEADERS_ENABLE_MODULE OFF"
)

# DirectX-Headers
CPMAddPackage(
    NAME DirectX-Headers
    GITHUB_REPOSITORY "microsoft/DirectX-Headers"
    GIT_TAG "v1.619.5" 
)
set_property(TARGET "DirectX-Headers" PROPERTY FOLDER "ThirdParty")
set_property(TARGET "DirectX-Guids" PROPERTY FOLDER "ThirdParty")

# SDL3 -> with features we use from C# side
if (NOT (EMSCRIPTEN OR ANDROID))
    CPMAddPackage(
        NAME SDL
        GITHUB_REPOSITORY libsdl-org/SDL
        GIT_TAG release-3.4.18
        OPTIONS
            "SDL_SHARED ON"
            "SDL_STATIC OFF"
            "SDL_TEST_LIBRARY OFF"
            "SDL_DISABLE_INSTALL ON"
            "SDL_DISABLE_INSTALL_DOCS ON"
            # Enable the subsystems we care about.
            "SDL_TIMERS ON"
            "SDL_AUDIO ON"
            "SDL_POWER ON"
            # Disable everything we don't care about.
            "SDL_ATOMIC OFF"
            "SDL_RENDER OFF"
            "SDL_HAPTIC OFF"
            "SDL_FILE OFF"
            "SDL_CPUINFO OFF"
            "SDL_FILESYSTEM OFF"
            "SDL_SENSOR OFF"
            "SDL_LOCALE OFF"
            "SDL_MISC OFF"
            "SDL_CAMERA OFF"
            "SDL_DIALOG OFF"
            "SDL_GPU OFF"
            "SDL_TRAY OFF"
    )

    # Install
    if (ALIMER_INSTALL)
        install(TARGETS "SDL3-shared"
            ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION  ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
        )
    endif()
    
    set_property(TARGET "SDL_uclibc" PROPERTY FOLDER "ThirdParty")
    set_property(TARGET "SDL3-shared" PROPERTY FOLDER "ThirdParty")
endif ()

# Add box3d dependency
CPMAddPackage(
    NAME box3d
    GITHUB_REPOSITORY erincatto/box3d
    GIT_TAG main
    OPTIONS
        "BUILD_SHARED_LIBS ON"
        "BOX3D_DOUBLE_PRECISION OFF"
)
set_property(TARGET box3d PROPERTY FOLDER "ThirdParty")

# Add box2d dependency
CPMAddPackage(
    NAME box2d
    GITHUB_REPOSITORY erincatto/box2d
    GIT_TAG "v3.2.0"
    OPTIONS
        "BUILD_SHARED_LIBS ON"
        "BOX2D_DOUBLE_PRECISION OFF"
)
set_property(TARGET box2d PROPERTY FOLDER "ThirdParty")
