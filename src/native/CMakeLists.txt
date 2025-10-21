cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INSTALL_MESSAGE LAZY)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_DEBUG_POSTFIX "_d")

# Set default symbol visibility to hidden. This prevents all Jolt's symbols to be exported on macOS. This can cause crashes if a program loads multiple shared libraries with different versions of Jolt.
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_OSX_DEPLOYMENT_TARGET "11" CACHE STRING "Minimum macOS deployment version")
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for OS X")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Build version
set(ALIMER_VERSION_MAJOR 1)
set(ALIMER_VERSION_MINOR 0)
set(ALIMER_VERSION_PATCH 0)
set(ALIMER_VERSION ${ALIMER_VERSION_MAJOR}.${ALIMER_VERSION_MINOR}.${ALIMER_VERSION_PATCH})

# Project declaration
project(Alimer.Native
    VERSION ${ALIMER_VERSION}
    DESCRIPTION "Cross platform game engine"
    HOMEPAGE_URL "https://github.com/amerkoleci/alimer"
    LANGUAGES C CXX
)

if (MSVC AND ALIMER_MASTER_PROJECT)
    if (NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
        set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif ()
endif ()

# Add the cmake directory to the module path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include common utilities
include(GNUInstallDirs)
include(CMakeDependentOption)
include(FetchPackage)
include(CPM)

# Define standard configurations (Debug, Release, Distribution)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Distribution")

# Set the default CMAKE_BUILD_TYPE to Release.
# This should be done before the project command since the latter can set
# CMAKE_BUILD_TYPE itself (it does so for nmake).
if (ALIMER_MASTER_PROJECT AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The default build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})
endif ()

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

# Setting Platform
if(EMSCRIPTEN)
    set(ALIMER_PLATFORM_NAME "Web")
elseif(ANDROID)
    set(ALIMER_PLATFORM_NAME "Android")
elseif(IOS)
    set(ALIMER_PLATFORM_NAME "iOS")
elseif(APPLE)
    set(ALIMER_PLATFORM_NAME "macOS")
elseif(WIN32)
    set(ALIMER_PLATFORM_NAME "Win32")
elseif(UNIX)
    set(ALIMER_PLATFORM_NAME "Unix")
else()
    message(FATAL_ERROR "Unrecognized platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Setting Architecture
if (DEFINED VCPKG_TARGET_ARCHITECTURE)
    set(ALIMER_ARCH ${VCPKG_TARGET_ARCHITECTURE})
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Ww][Ii][Nn]32$")
    set(ALIMER_ARCH x86)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Xx]64$")
    set(ALIMER_ARCH x64)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Aa][Rr][Mm]$")
    set(ALIMER_ARCH arm)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Aa][Rr][Mm]64$")
    set(ALIMER_ARCH arm64)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Aa][Rr][Mm]64EC$")
    set(ALIMER_ARCH arm64ec)
elseif(CMAKE_VS_PLATFORM_NAME_DEFAULT MATCHES "^[Ww][Ii][Nn]32$")
    set(ALIMER_ARCH x86)
elseif(CMAKE_VS_PLATFORM_NAME_DEFAULT MATCHES "^[Xx]64$")
    set(ALIMER_ARCH x64)
elseif(CMAKE_VS_PLATFORM_NAME_DEFAULT MATCHES "^[Aa][Rr][Mm]$")
    set(ALIMER_ARCH arm)
elseif(CMAKE_VS_PLATFORM_NAME_DEFAULT MATCHES "^[Aa][Rr][Mm]64$")
    set(ALIMER_ARCH arm64)
elseif(CMAKE_VS_PLATFORM_NAME_DEFAULT MATCHES "^[Aa][Rr][Mm]64EC$")
    set(ALIMER_ARCH arm64ec)
endif()

# Determines host architecture
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "[Aa][Rr][Mm]64|aarch64|arm64")
    set(ALIMER_HOST_ARCH arm64)
else()
    set(ALIMER_HOST_ARCH x64)
endif()

# Detect use of the clang-cl.exe frontend, which does not support all of clangs normal options
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if ("${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
        set(MSVC_CLANG TRUE)
    endif()
endif()

# Library type
if (IOS OR EMSCRIPTEN)
    set(ALIMER_BUILD_SHARED OFF CACHE BOOL "Always Disable shared library on (IOS, WEB)" FORCE)
elseif(ANDROID)
    set(ALIMER_BUILD_SHARED ON CACHE BOOL "Always enable shared library on (ANDROID)" FORCE)
else ()
    option(ALIMER_BUILD_SHARED "Build engine as shared library" ON)
endif ()

option(ALIMER_ENABLE_ASAN_UBSAN "Enable Address and Undefined Behavior Sanitizers" OFF)
option(ALIMER_ENABLE_TSAN "Enable Thread Sanitizer" OFF)

option(ALIMER_INSTALL "Generate the install target" ${ALIMER_MASTER_PROJECT})
option(ALIMER_SAMPLES "Build samples" ${ALIMER_MASTER_PROJECT})

option(ALIMER_USE_SDL "Use SDL for desktop windows" ON)

option(ALIMER_AUDIO "Enable Audio support" ON)
option(ALIMER_GPU "Enable GPU support" ON)
option(ALIMER_PHYSICS "Enable Physics support (using Jolt)" ON)

# Determine available GPU backends
if(EMSCRIPTEN)
    set(ALIMER_GPU_HAS_D3D12 OFF)
    set(ALIMER_GPU_HAS_VULKAN OFF)
    set(ALIMER_GPU_HAS_METAL OFF)
    set(ALIMER_GPU_HAS_WEBGPU ON) 
elseif (WIN32)
    set(ALIMER_GPU_HAS_D3D12 OFF)  # Disable for now
    set(ALIMER_GPU_HAS_VULKAN ON)
    set(ALIMER_GPU_HAS_METAL OFF)
    set(ALIMER_GPU_HAS_WEBGPU OFF) # Disable for now
elseif(APPLE)
    set(ALIMER_GPU_HAS_D3D12 OFF)
    set(ALIMER_GPU_HAS_VULKAN OFF)
    set(ALIMER_GPU_HAS_METAL ON)
    set(ALIMER_GPU_HAS_WEBGPU OFF)
elseif(UNIX OR ANDROID)
    set(ALIMER_GPU_HAS_D3D12 OFF)
    set(ALIMER_GPU_HAS_VULKAN ON)
    set(ALIMER_GPU_HAS_METAL OFF)
    set(ALIMER_GPU_HAS_WEBGPU OFF)
endif()

cmake_dependent_option(ALIMER_GPU_VULKAN "Enable Vulkan GPU backend" ON "ALIMER_GPU_HAS_VULKAN" OFF)
cmake_dependent_option(ALIMER_GPU_D3D12 "Enable D3D12 GPU backend" ON "ALIMER_GPU_HAS_D3D12" OFF)
cmake_dependent_option(ALIMER_GPU_METAL "Enable Metal GPU backend" ON "ALIMER_GPU_HAS_METAL" OFF)
cmake_dependent_option(ALIMER_GPU_WEBGPU "Enable WebGPU GPU backend" ON "ALIMER_GPU_HAS_WEBGPU" OFF)

# Metal C++ headers
if(NOT DEFINED ALIMER_METAL_CPP_URL)
    set(ALIMER_METAL_CPP_URL "https://developer.apple.com/metal/cpp/files/metal-cpp_macOS15.2_iOS18.2.zip")
endif()

# Set general compiler flags
if (MSVC)
    # Multiprocessor compiler
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /JMC- /Zc:__cplusplus  /Zc:inline")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gm- /nologo /diagnostics:classic /FC /fp:except-")

    # Set compiler flag for disabling RTTI
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")

    # Disable exceptions
	# Remove any existing compiler flag that enables exceptions
	string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

	# Disable warning about STL and compiler-generated types using noexcept when exceptions are disabled
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4577")

    # Disable warning about C++ exception handler used, but unwind semantics not enabled
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4530")

    if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
      # Enable function-level linking
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gy /Gw")
    endif()

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
    # Set compiler flag for disabling RTTI
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

	# Set compiler flag for disabling exception-handling
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

    if (ALIMER_ENABLE_ASAN_UBSAN)
       set(EXTRA_SANITIZE_OPTIONS "-fsanitize=address -fsanitize=undefined,implicit-conversion,float-divide-by-zero,local-bounds -fno-sanitize-recover=all")
    endif()
    if (ALIMER_ENABLE_TSAN)
       set(EXTRA_SANITIZE_OPTIONS "-fsanitize=thread")
    endif()

    if (EMSCRIPTEN)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
        #if(ALIMER_ENABLE_THREAD)
        #    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
        #    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
        #endif()
    elseif(ANDROID)
    endif()

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${EXTRA_SANITIZE_OPTIONS}")
endif ()

set(CMAKE_CXX_FLAGS_DISTRIBUTION "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS_DISTRIBUTION "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_DISTRIBUTION "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

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
    set(ALIMER_LIBRARY_TYPE SHARED)
    message(STATUS "  Library         SHARED")
else ()
    set(ALIMER_LIBRARY_TYPE STATIC)
    message(STATUS "  Library         STATIC")
endif ()

message(STATUS "  Audio           ${ALIMER_AUDIO}")

# GPU
message(STATUS "  GPU             ${ALIMER_GPU}")
if (ALIMER_GPU_VULKAN)
    message(STATUS "      - Vulkan:      ALIMER_GPU_VULKAN")
endif()
if (ALIMER_GPU_D3D12)
    message(STATUS "      - D3D12:       ALIMER_GPU_D3D12")
endif ()
if (ALIMER_GPU_METAL)
    message(STATUS "      - Metal:       ALIMER_GPU_METAL")
endif ()
if (ALIMER_GPU_WEBGPU)
    message(STATUS "      - WebGPU:      ALIMER_GPU_WEBGPU")
endif()

message(STATUS "  Physics         ${ALIMER_PHYSICS}")
message(STATUS "  Install         ${ALIMER_INSTALL}")
message(STATUS "  Samples         ${ALIMER_SAMPLES}")

add_subdirectory(third_party)
add_subdirectory(engine)
add_subdirectory(samples)
