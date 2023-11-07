cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not supported. Please choose a different binary directory.")
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

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment version")
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
project(Alimer
    VERSION ${ALIMER_VERSION}
    DESCRIPTION "Cross platform game engine"
    HOMEPAGE_URL "https://github.com/amerkoleci/alimer"
    LANGUAGES C CXX
)

# Set the default CMAKE_BUILD_TYPE to Release.
# This should be done before the project command since the latter can set
# CMAKE_BUILD_TYPE itself (it does so for nmake).
if (ALIMER_MASTER_PROJECT AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The default build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})
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
elseif(WINDOWS_STORE)
    set(ALIMER_PLATFORM_NAME "UWP")
elseif(WIN32)
    set(ALIMER_PLATFORM_NAME "Win32")
elseif(UNIX)
    set(ALIMER_PLATFORM_NAME "Unix")
    set(ALIMER_LINUX_WSI_SELECTION "XCB" CACHE STRING "Select WSI target (XCB, XLIB, WAYLAND, D2D)")
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
endif()

# Options
if (IOS OR EMSCRIPTEN)
    set(ALIMER_SHARED_LIBRARY OFF CACHE BOOL "Disable shared library on (IOS, WEB)" FORCE)
elseif(ANDROID)
    set(ALIMER_SHARED_LIBRARY ON CACHE BOOL "Always enable shared library on (ANDROID)" FORCE)    
else()
    option(ALIMER_SHARED_LIBRARY "Build as shared library" ON)
endif ()

option(ALIMER_AUDIO "Enable Audio library" ON)
option(ALIMER_PHYSICS "Enable Physics library (using Jolt)" ON)

option(ALIMER_VULKAN "Make Vulkan backend available" ON)
if (WIN32 OR WINDOWS_STORE)
	option(ALIMER_D3D12 "Make D3D12 backend available" ON)
endif()

option(ALIMER_USE_GLFW "Use GLFW for desktop windows" ON)

option(ALIMER_INSTALL "Generate the install target" ${ALIMER_MASTER_PROJECT})
option(ALIMER_SAMPLES "Build samples" ${ALIMER_MASTER_PROJECT})

# add dependencies
include(cmake/CPM.cmake)
include(GNUInstallDirs)

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

if (ALIMER_SHARED_LIBRARY)
    set(ALIMER_LIBRARY_TYPE SHARED)
    message(STATUS "  Library         SHARED")
else ()
    set(ALIMER_LIBRARY_TYPE STATIC)
    message(STATUS "  Library         STATIC")
endif ()

message(STATUS "  Audio           ${ALIMER_AUDIO}")
message(STATUS "  Physics         ${ALIMER_PHYSICS}")

# GPU
if (ALIMER_VULKAN)
    message(STATUS "      - Vulkan:      ALIMER_VULKAN")
endif()
if (ALIMER_D3D12)
    if(ALIMER_D3D12_AGILITY_SDK)
        message(STATUS "      - Direct3D12 (Agility SDK):  ALIMER_D3D12")
    else()
        message(STATUS "      - Direct3D12:  ALIMER_D3D12")
    endif ()
endif ()

message(STATUS "  Install         ${ALIMER_INSTALL}")
message(STATUS "  Samples         ${ALIMER_SAMPLES}")

# ThirdParty libs
#add_subdirectory (third_party)

if(ALIMER_USE_GLFW AND NOT (EMSCRIPTEN OR ANDROID OR WINDOWS_STORE))
    CPMAddPackage(
        NAME glfw
        GITHUB_REPOSITORY glfw/glfw
        GIT_TAG 3eaf1255b29fdf5c2895856c7be7d7185ef2b241
        OPTIONS
            "GLFW_BUILD_TESTS OFF"
            "GLFW_BUILD_EXAMPLES OFF"
            "GLFW_BUILD_DOCS OFF"
            "GLFW_INSTALL OFF"
        EXCLUDE_FROM_ALL YES
    )
endif ()

# Engine
add_subdirectory (engine)

# Samples
add_subdirectory (samples)
