# CMake version
cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not supported. Please choose a different binary directory.")
	return()
endif()

# Only 64 bit is supported
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(FATAL_ERROR "32 bit isn't supported")
    return()
endif()

# Determine if engine is built as a subproject (using add_subdirectory)
# or if it is the master project.
if (NOT DEFINED ALIMER_MASTER_PROJECT)
    set(ALIMER_MASTER_PROJECT OFF)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(ALIMER_MASTER_PROJECT ON)
        message(STATUS "CMake version: ${CMAKE_VERSION}")
    endif ()
endif ()

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Configure CMake global variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INSTALL_MESSAGE LAZY)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_DEBUG_POSTFIX "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Enable IPO with its policy
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(CMAKE_OSX_DEPLOYMENT_TARGET "11" CACHE STRING "Minimum OS X deployment version")
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for OS X")

# Build version
set(ALIMER_VERSION_MAJOR 1)
set(ALIMER_VERSION_MINOR 0)
set(ALIMER_VERSION_PATCH 0)
set(ALIMER_VERSION ${ALIMER_VERSION_MAJOR}.${ALIMER_VERSION_MINOR}.${ALIMER_VERSION_PATCH})

# Project declaration
project (Alimer
    VERSION ${ALIMER_VERSION}
    DESCRIPTION "Cross platform game engine"
    HOMEPAGE_URL "https://github.com/amerkoleci/alimer_new"
    LANGUAGES C CXX
)

if (ALIMER_MASTER_PROJECT AND NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

# Add the cmake directory to the module path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include common utilities
include(GNUInstallDirs)
include(CMakeDependentOption)
include(FetchPackage)
include(AlimerUtils)
include(Alimer)

# Define standard configurations (Debug, Release, Distribution)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Distribution")

# Set the default CMAKE_BUILD_TYPE to Release.
# This should be done before the project command since the latter can set
# CMAKE_BUILD_TYPE itself (it does so for nmake).
if (ALIMER_MASTER_PROJECT AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The default build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})
endif ()

set(ALIMER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(ALIMER_ASSETS_DIR "${ALIMER_ROOT_DIR}/Assets" CACHE PATH "" FORCE)
set(ALIMER_SHADERS_INCLUDE_DIR "${ALIMER_ROOT_DIR}/Assets/Shaders" CACHE PATH "" FORCE)
set(ALIMER_SHADERS_DEFINITIONS_DIR "${ALIMER_ROOT_DIR}/Source" CACHE PATH "" FORCE)
set(ALIMER_SHADERS_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>/CompiledShaders")

mark_as_advanced(ALIMER_ROOT_DIR)
mark_as_advanced(ALIMER_ASSETS_DIR)
mark_as_advanced(ALIMER_SHADERS_INCLUDE_DIR)
mark_as_advanced(ALIMER_SHADERS_DEFINITIONS_DIR)

execute_process(COMMAND git rev-list --count HEAD
    OUTPUT_VARIABLE ALIMER_REVISION_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if ("${ALIMER_REVISION_ID}" STREQUAL "")
    # We don't have any source control so just hard-code all of this information to empty
    set (ALIMER_REVISION_ID 0)
    set (ALIMER_SHORT_CHANGE_SET 0)
    set (ALIMER_CHANGE_SET 0)
    set (ALIMER_CHANGE_SET_DATE "")
else ()
    execute_process(COMMAND git rev-parse --short HEAD
        OUTPUT_VARIABLE ALIMER_SHORT_CHANGE_SET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(COMMAND git rev-parse HEAD
        OUTPUT_VARIABLE ALIMER_CHANGE_SET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(COMMAND git log -1 --abbrev=12 --date=format:%Y-%m-%d --pretty=format:%cd
        OUTPUT_VARIABLE ALIMER_CHANGE_SET_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif ()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")

# Library type
if (IOS OR EMSCRIPTEN)
    set(ALIMER_BUILD_SHARED OFF CACHE BOOL "Always Disable shared library on (IOS, WEB)" FORCE)
elseif(ANDROID)
    set(ALIMER_BUILD_SHARED ON CACHE BOOL "Always enable shared library on (ANDROID)" FORCE)
else ()
    option(ALIMER_BUILD_SHARED "Build engine as shared library" ON)
endif ()

# Options
option(ALIMER_INSTALL "Generate the install target" ${ALIMER_MASTER_PROJECT})
option(ALIMER_SAMPLES "Build samples" ${ALIMER_MASTER_PROJECT})

option(ALIMER_STD_CONTAINERS "Use standard containers (std::vector, std::unordered_map etc. instead of own containers)" OFF)
option(ALIMER_PROFILING "Enable performance profiling" ON)
#option(ALIMER_NETWORK "Enable Networking module " ON)
#option(ALIMER_IMGUI "Enable ImGui module" ON)
#option(ALIMER_2D "Enable 2D module" OFF) # WIP

# Enable SIMD for the WASM build. Note that this is currently off by default since not all browsers support this.
# See: https://caniuse.com/?search=WebAssembly%20SIMD (Safari got support in March 2023 and was the last major browser to get support).
option(ALIMER_USE_WASM_SIMD "Enable SIMD for WASM" OFF)

option(ALIMER_ENABLE_ASAN_UBSAN "Enable Address and Undefined Behavior Sanitizers" OFF)
option(ALIMER_ENABLE_TSAN "Enable Thread Sanitizer" OFF)

# Determine available RHI backends
if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    set(ALIMER_RHI_HAS_D3D12 OFF)
    set(ALIMER_RHI_HAS_VULKAN OFF)
    set(ALIMER_RHI_HAS_METAL OFF)
    set(ALIMER_RHI_HAS_WEBGPU ON)
elseif (WIN32)
    set(ALIMER_RHI_HAS_D3D12 ON)
    set(ALIMER_RHI_HAS_VULKAN ON)
    set(ALIMER_RHI_HAS_METAL OFF)
    set(ALIMER_RHI_HAS_WEBGPU OFF) # TODO: Enable WebGPU on Windows when backend is ready
elseif(APPLE)
    set(ALIMER_RHI_HAS_D3D12 OFF)
    set(ALIMER_RHI_HAS_VULKAN ON)
    set(ALIMER_RHI_HAS_METAL ON)
    set(ALIMER_RHI_HAS_WEBGPU OFF)
elseif(UNIX OR ANDROID)
    set(ALIMER_RHI_HAS_D3D12 OFF)
    set(ALIMER_RHI_HAS_VULKAN ON)
    set(ALIMER_RHI_HAS_METAL OFF)
    set(ALIMER_RHI_HAS_WEBGPU OFF)
endif()

# RHI backends
cmake_dependent_option(ALIMER_RHI_VULKAN "Enable Vulkan backend" ON "ALIMER_RHI_HAS_VULKAN" OFF)
cmake_dependent_option(ALIMER_RHI_D3D12 "Enable D3D12 backend" ON "ALIMER_RHI_HAS_D3D12" OFF)
cmake_dependent_option(ALIMER_D3D12_AGILITY_SDK "Enable Direct3D12 Agility SDK" OFF "ALIMER_RHI_HAS_D3D12" OFF)
cmake_dependent_option(ALIMER_RHI_METAL "Enable Metal backend" ON "ALIMER_RHI_HAS_METAL" OFF)
cmake_dependent_option(ALIMER_RHI_WEBGPU "Enable WebGPU backend" ON "ALIMER_RHI_HAS_WEBGPU" OFF)

# Metal C++ headers
if(NOT DEFINED ALIMER_RHI_METAL_CPP_URL)
    set(ALIMER_RHI_METAL_CPP_URL "https://developer.apple.com/metal/cpp/files/metal-cpp_macOS15.2_iOS18.2.zip")
endif()

# ShaderCompiler
#if ((WIN32 OR UNIX) AND NOT (APPLE))
if (WIN32)
    option(ALIMER_SHADER_COMPILER "Build the shader compiler executable" ${ALIMER_MASTER_PROJECT})
else()
    set(ALIMER_SHADER_COMPILER OFF CACHE INTERNAL "Disable shader compiler" FORCE)
endif ()

# Editor
#if (ANDROID OR IOS OR EMSCRIPTEN)
    set(ALIMER_CSHARP_BINDINGS OFF CACHE INTERNAL "Disable C# Bindings" FORCE)
    set(ALIMER_CSHARP OFF CACHE INTERNAL "Disable C# support" FORCE)
    set(ALIMER_EDITOR OFF CACHE INTERNAL "Disable Editor" FORCE)
#else()
#    option(ALIMER_CSHARP_BINDINGS "Enable C# bindings" ${ALIMER_MASTER_PROJECT})
#    option(ALIMER_CSHARP "Enable C# support" ${ALIMER_MASTER_PROJECT})
#    option(ALIMER_EDITOR "Enable Editor" ${ALIMER_MASTER_PROJECT})
#endif()

# Tests
if (ANDROID OR IOS OR EMSCRIPTEN)
    set(ALIMER_TESTS OFF CACHE INTERNAL "Disable tests" FORCE)
else()
    option(ALIMER_TESTS "Enable tests" ${ALIMER_MASTER_PROJECT})
endif()

# Set general compiler flags
if (MSVC)
     # Multiprocessor compiler
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

    # /permissive- (Standards conformance)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")

    # /JMC (Debug just my code)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /JMC-")

    # /Zc:__cplusplus (Enable updated __cplusplus macro)
    # /Zc:inline (Remove unreferenced COMDAT)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /Zc:inline")

    # /Zc:twoPhase- (disable two-phase name lookup)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:twoPhase-")

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.26)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:preprocessor")
    endif()

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.28)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:lambda")
    endif()

    # /Gm (Enable Minimal Rebuild)
    # /nologo (Suppress Startup Banner) (C/C++)
    # /diagnostics (Compiler diagnostic options)
    # /FC (Full path of source code file in diagnostics)
    # /fp (Specify floating-point behavior)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gm- /nologo /diagnostics:classic /FC /fp:except-")

    if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
      # Enable function-level linking
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gy /Gw")
    endif()

    # Use fast floating point model
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast")

    set(CMAKE_CXX_FLAGS_DISTRIBUTION "${CMAKE_CXX_FLAGS_RELEASE}")
    
    if (ALIMER_ENABLE_ASAN_UBSAN)
       set(EXTRA_SANITIZE_OPTIONS "-fsanitize=address -fsanitize=undefined,implicit-conversion,float-divide-by-zero,local-bounds -fno-sanitize-recover=all")
    endif()
    if (ALIMER_ENABLE_TSAN)
       set(EXTRA_SANITIZE_OPTIONS "-fsanitize=thread")
    endif()

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${EXTRA_SANITIZE_OPTIONS}")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /showFilenames")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments") # Clang emits warnings about unused arguments such as /MP and /GL

        #set(CMAKE_SHARED_LINKER_FLAGS_RELEASEASAN "/LIBPATH:${CLANG_LIB_PATH} clang_rt.asan-x86_64.lib -wholearchive:clang_rt.asan-x86_64.lib clang_rt.asan_cxx-x86_64.lib -wholearchive:clang_rt.asan_cxx-x86_64.lib")
		#set(CMAKE_SHARED_LINKER_FLAGS_RELEASEUBSAN "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LIBPATH:${CLANG_LIB_PATH}")
		#set(CMAKE_EXE_LINKER_FLAGS_RELEASEASAN "${CMAKE_SHARED_LINKER_FLAGS_RELEASEASAN}")
		#set(CMAKE_EXE_LINKER_FLAGS_RELEASEUBSAN "${CMAKE_SHARED_LINKER_FLAGS_RELEASEUBSAN}")
    endif ()
else()
    set(CMAKE_CXX_FLAGS_DISTRIBUTION "${CMAKE_CXX_FLAGS_RELEASE}")

    if (ALIMER_ENABLE_ASAN_UBSAN)
       set(EXTRA_SANITIZE_OPTIONS "-fsanitize=address -fsanitize=undefined,implicit-conversion,float-divide-by-zero,local-bounds -fno-sanitize-recover=all")
    endif()
    if (ALIMER_ENABLE_TSAN)
       set(EXTRA_SANITIZE_OPTIONS "-fsanitize=thread")
    endif()

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${EXTRA_SANITIZE_OPTIONS}")
endif ()

set(CMAKE_SHARED_LINKER_FLAGS_DISTRIBUTION "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_DISTRIBUTION "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

# Enable testing
if (ALIMER_TESTS)
    include (CTest)
endif ()

if(ALIMER_SHADER_COMPILER)
    #include(cmake/AlimerShaderCompiler.cmake)
endif ()

if (ALIMER_EDITOR)
    #include(cmake/AlimerAssetCompiler.cmake)
endif ()

# Assets
add_subdirectory(Assets)

# Source
add_subdirectory(Source)

# Print current build configuration
message(STATUS "Engine version: ${ALIMER_VERSION}")
message(STATUS "GIT Revision ID: ${ALIMER_REVISION_ID}")
message(STATUS "GIT Short ChangeSet: ${ALIMER_SHORT_CHANGE_SET}")
message(STATUS "GIT ChangeSet: ${ALIMER_CHANGE_SET}")
message(STATUS "GIT ChangeSet Date: ${ALIMER_CHANGE_SET_DATE}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

message(STATUS "Detected platform: ${ALIMER_PLATFORM_NAME}")
message(STATUS "Detected architecture: ${ALIMER_ARCH}")
message(STATUS "Detected host architecture: ${ALIMER_HOST_ARCH}")
    
if (ALIMER_BUILD_SHARED)
  message(STATUS "  Library         SHARED")
else ()
  message(STATUS "  Library         STATIC")
endif ()

message(STATUS "  Std Containers  ${ALIMER_STD_CONTAINERS}")
message(STATUS "  Profiling       ${ALIMER_PROFILING}")
#message(STATUS "  Network         ${ALIMER_NETWORK}")
#message(STATUS "  ImGui           ${ALIMER_IMGUI}")
#message(STATUS "  2D              ${ALIMER_2D}")

message(STATUS "  RHI backends")
if (ALIMER_RHI_D3D12)
    if(ALIMER_D3D12_AGILITY_SDK)
        message(STATUS "      - Direct3D12 (Agility SDK):  ALIMER_RHI_D3D12")
    else()
        message(STATUS "      - Direct3D12:  ALIMER_RHI_D3D12")
    endif ()
endif ()
if (ALIMER_RHI_VULKAN)
    message(STATUS "      - Vulkan:      ALIMER_RHI_VULKAN")
endif()
if (ALIMER_RHI_METAL)
    message(STATUS "      - Metal:       ALIMER_RHI_METAL")
endif()
if (ALIMER_RHI_WEBGPU)
    message(STATUS "      - WebGPU:      ALIMER_RHI_WEBGPU")
endif()

message(STATUS "  ShaderCompiler  ${ALIMER_SHADER_COMPILER}")
message(STATUS "  Editor          ${ALIMER_EDITOR}")
message(STATUS "  C#              ${ALIMER_CSHARP}")

message(STATUS "  Install         ${ALIMER_INSTALL}")
message(STATUS "  Samples         ${ALIMER_SAMPLES}")
message(STATUS "  Tests           ${ALIMER_TESTS}")

# README and LICENSE
if(ALIMER_INSTALL)
    message(STATUS "Generating install")
    install (FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
        DESTINATION .
    )

    # Install data files
    file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    if (NOT ANDROID)
        foreach(dir Assets)
            foreach (CONFIG ${CMAKE_CONFIGURATION_TYPES})
                create_symlink(${CMAKE_CURRENT_SOURCE_DIR}/${dir} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}/${dir})
            endforeach()

            # Install
            install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir} DESTINATION ${CMAKE_INSTALL_BINDIR})
        endforeach()
    endif ()

    # DXC
    if (ALIMER_SHADER_COMPILER AND TARGET ShaderMake)
        if (WIN32)
            set(DXIL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/Source/ThirdParty/dxc/bin/win-x64/dxil.dll")
	        set(DXCOMPILER_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/Source/ThirdParty/dxc/bin/win-x64/dxcompiler.dll")
        elseif(UNIX)
            set(DXIL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/Source/ThirdParty/dxc/bin/linux-x64/libdxil.so")
	        set(DXCOMPILER_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/Source/ThirdParty/dxc/linux/win-x64/libdxcompiler.so")
        endif()

	    install(FILES ${DXIL_LIBRARY} ${DXCOMPILER_LIBRARY} DESTINATION ${CMAKE_INSTALL_BINDIR})

        install(
            TARGETS ShaderMake
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )

        install(DIRECTORY ${ALIMER_SHADERS_OUTPUT_DIR} DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif ()
endif ()

if (ALIMER_EDITOR AND ALIMER_CSHARP)
   # set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Alimer.Editor)
endif ()

