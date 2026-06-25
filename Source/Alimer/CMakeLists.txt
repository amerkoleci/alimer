# Define target name
set (TARGET_NAME Alimer)

macro (define_engine_source_files)
	cmake_parse_arguments(DEFINE_SRC_FILES "NO_INSTALL" "" "" ${ARGN} )

	foreach (path ${DEFINE_SRC_FILES_UNPARSED_ARGUMENTS})
		# Get header files
		file (GLOB _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.h ${path}/*.inl)
		list (APPEND HEADER_FILES ${_files})

		# Install them
		file (GLOB INSTALL_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.h ${path}/*.inl)

		if (ALIMER_INSTALL AND NOT DEFINE_SRC_FILES_NO_INSTALL)
			install (
                FILES ${INSTALL_FILES} 
			    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${path}
			)
		endif ()

		# Get source files
		file (GLOB _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.c ${path}/*.cpp)
		list (APPEND SOURCE_FILES ${_files})
	endforeach ()
endmacro()

# Configure file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AlimerConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/AlimerConfig.h)

# Add root-level files
set (ROOT_HEADER_FILES ${CMAKE_CURRENT_BINARY_DIR}/AlimerConfig.h)
file (GLOB ROOT_HEADER_FILES *.h)
file (GLOB SOURCE_FILES *.c *.cpp)
source_group("" FILES ${ROOT_HEADER_FILES})
source_group("" FILES ${SOURCE_FILES})

set (HEADER_FILES ${ROOT_HEADER_FILES})

define_engine_source_files(
    Core
    Platform
    Math
    IO
    Serialization
    Assets
    Scene
    RHI
    Animations
    Audio
    Audio/Components
    Shaders
    Renderer
    Renderer/Components
    Physics
    Physics/Components
    UI
    ImGui
)

# Platform/Private files (not installed)
define_engine_source_files(
    Private
    Platform/${ALIMER_PLATFORM_NAME}
    RHI/Private
    NO_INSTALL
)

# SDL
if (NOT (EMSCRIPTEN OR ANDROID OR WINDOWS_STORE))
    define_engine_source_files(Platform/SDL NO_INSTALL)
endif ()

# RHI
if (ALIMER_RHI_D3D12)
    define_engine_source_files(RHI/D3D12 NO_INSTALL)
endif()

if (ALIMER_RHI_VULKAN)
    define_engine_source_files(RHI/Vulkan NO_INSTALL)
endif()

if (ALIMER_RHI_METAL)
    define_engine_source_files(RHI/Metal NO_INSTALL)
endif()

# Jolt Physics
define_engine_source_files(Physics/Jolt NO_INSTALL)

group_sources()

if (ALIMER_BUILD_SHARED)
    add_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${MISC_FILES})

    target_compile_definitions(${TARGET_NAME}
        PRIVATE -DALIMER_IMPLEMENTATION=1 -DALIMER_EXPORTS=1 -DIMGUI_EXPORTS=1
  	    PUBLIC -DALIMER_IMPORTS=1
    )
else()
    add_library(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${MISC_FILES})
    target_compile_definitions (${TARGET_NAME} PUBLIC -DALIMER_STATIC=1)
endif ()

# pch
target_precompile_headers(${TARGET_NAME} PRIVATE pch.h)

# Include directories
target_include_directories(${TARGET_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>        # For parent projects to find auto-generated engine headers
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>          # Misc configured headers
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>          # Source folder
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>       # For parent projects to find engine headers
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ImGui>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ThirdParty>
)

if (ALIMER_SHADER_COMPILER)
    add_dependencies(${TARGET_NAME} AlimerShaders)
endif ()

# Compiler-specific setup
if (MSVC)
    # Debug information
	target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:/Zi>)

    # Enable full optimization in dev/release
	target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:/Od> $<$<NOT:$<CONFIG:Debug>>:/Ox>)

    # Inline function expansion
	target_compile_options(${TARGET_NAME} PRIVATE /Ob2)

    # Enable intrinsic functions in dev/release
	target_compile_options(${TARGET_NAME} PRIVATE $<$<NOT:$<CONFIG:Debug>>:/Oi>)

    # Favor fast code
	target_compile_options(${TARGET_NAME} PRIVATE /Ot)

    # Enable fiber-safe optimizations in dev/release
	target_compile_options(${TARGET_NAME} PRIVATE $<$<NOT:$<CONFIG:Debug>>:/GT>)

	# Enable string pooling
	target_compile_options(${TARGET_NAME} PRIVATE /GF)

    # Use security checks only in debug
	target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:DEBUG>:/sdl> $<$<NOT:$<CONFIG:DEBUG>>:/sdl->)

    # Use fast floating point model
    target_compile_options(${TARGET_NAME} PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:/guard:cf>)

    # Set warning level 3
    target_compile_options(${TARGET_NAME} PRIVATE /W3)

    # Warnings as Errors Enabled
    target_compile_options(${TARGET_NAME} PRIVATE /WX)

    # Enable hot reload
    target_link_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)

    if (ALIMER_BUILD_ENABLE_CODE_ANALYSIS)
        target_compile_options(${TARGET_NAME} PRIVATE /analyze)
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.24)
        target_compile_options(${TARGET_NAME} PRIVATE /ZH:SHA_256)
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.26)
        target_compile_options(${TARGET_NAME} PRIVATE /wd5104 /wd5105)
    endif()

    if(WINDOWS_STORE AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.32))
      target_compile_options(${TARGET_NAME} PRIVATE /wd5246)
    endif()

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.35)
      if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
        target_compile_options(${TARGET_NAME} PRIVATE $<$<NOT:$<CONFIG:Debug>>:/Zc:checkGwOdr>)
      endif()

      if(NOT (DEFINED XBOX_CONSOLE_TARGET))
        target_compile_options(${TARGET_NAME} PRIVATE $<$<VERSION_GREATER_EQUAL:${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION},10.0.22000>:/Zc:templateScope>)
      endif()
    endif()

else()
    target_compile_options(${TARGET_NAME} PRIVATE -Wall)

    if(ALIMER_COMPILER_CLANG AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0)
        target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-unsafe-buffer-usage")
    endif()
    
	# Set compiler flag for disabling exception-handling
	if (NOT ALIMER_EXCEPTIONS_ENABLED)
		target_compile_options(${PROJECT_NAME} PRIVATE -fno-exceptions)
	endif()

    # Use fast floating point model
    #target_compile_options(${TARGET_NAME} PRIVATE -ffast-math -fno-finite-math-only -ffp-contract=fast)
endif ()

if(WIN32)
    target_compile_definitions(${TARGET_NAME} PUBLIC _UNICODE UNICODE)
    target_compile_definitions(${TARGET_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif ()

# Platform specific settings and libraries
if (WIN32)
else()
    target_link_libraries (${TARGET_NAME} PUBLIC dl)
    if (ANDROID)
        target_link_libraries (${TARGET_NAME} PUBLIC log android native_app_glue)
    elseif (EMSCRIPTEN)
        target_link_options(${TARGET_NAME} PRIVATE -sALLOW_MEMORY_GROWTH)
        # Use GLFW
        target_link_options(${TARGET_NAME} PRIVATE -sUSE_GLFW)

        # Handle WebGPU symbols
        target_link_options(${TARGET_NAME} PRIVATE -sUSE_WEBGPU)

        # Enable the use of emscripten_sleep()
	    target_link_options(${TARGET_NAME} PRIVATE -sASYNCIFY)

        if (ALIMER_USE_WASM_SIMD)
			# See: https://emscripten.org/docs/porting/simd.html#webassembly-simd-intrinsics
			# Note that this does not require the browser to actually support SSE 4.2 it merely means that it can translate those instructions to WASM SIMD instructions
			target_compile_options(${TARGET_NAME} PUBLIC -msimd128 -msse4.2)
        endif ()
    else()
        target_link_libraries (${TARGET_NAME} PUBLIC m pthread)
    endif ()
endif ()

# Private link libraries
target_link_libraries(${TARGET_NAME} PRIVATE
    stb
    lz4
    rapidjson
    #ktx
    miniaudio
    cgltf
    Jolt
    ShaderMakeBlob
)

# Public link libraries
target_link_libraries(${TARGET_NAME} PUBLIC
    fmt::fmt
    spdlog
)

if (NOT ALIMER_STD_CONTAINERS)
    target_link_libraries(${TARGET_NAME} PUBLIC unordered_dense)
endif()

# SDL
if (NOT (EMSCRIPTEN OR ANDROID OR WINDOWS_STORE))
    #target_sources(${TARGET_NAME} PRIVATE
    #    ${CMAKE_CURRENT_SOURCE_DIR}/ImGui/backends/imgui_impl_sdl3.cpp
	#    ${CMAKE_CURRENT_SOURCE_DIR}/ImGui/backends/imgui_impl_sdl3.h
    #)

    target_link_libraries(${TARGET_NAME} PRIVATE SDL3-static)
    target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_USE_SDL)
endif ()

if (ALIMER_RHI_VULKAN)
    target_compile_definitions(${TARGET_NAME} PRIVATE
        $<$<BOOL:${WIN32}>:
            VK_USE_PLATFORM_WIN32_KHR
        >
        $<$<BOOL:${APPLE}>:
            VK_USE_PLATFORM_METAL_EXT;VK_ENABLE_BETA_EXTENSIONS
        >
        $<$<BOOL:${ALIMER_ENABLE_XLIB_SUPPORT}>:
            VK_USE_PLATFORM_XLIB_KHR
        >
        $<$<BOOL:${ALIMER_ENABLE_WAYLAND_SUPPORT}>:
            VK_USE_PLATFORM_WAYLAND_KHR
        >
    )
    target_link_libraries(${TARGET_NAME} PRIVATE vma)
endif (ALIMER_RHI_VULKAN)

if (ALIMER_RHI_D3D12)
    target_link_libraries(${TARGET_NAME} PRIVATE d3d12ma)
endif (ALIMER_RHI_D3D12)

if(ALIMER_RHI_METAL)
    target_link_libraries(${TARGET_NAME} PRIVATE alimer-rhi-metal-cpp)
endif ()

# Install root header files and config file
if(ALIMER_INSTALL)
    install (FILES 
        ${ROOT_HEADER_FILES}
        ${CMAKE_CURRENT_BINARY_DIR}/AlimerConfig.h 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    )

    install(TARGETS ${TARGET_NAME}
        EXPORT ${TARGET_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()
