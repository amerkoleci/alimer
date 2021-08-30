# CMake version
cmake_minimum_required (VERSION 3.14)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not supported. Please choose a different binary directory.")
	return()
endif()

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Configure CMake global variables
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Project declaration
project (Alimer)

# Build version
set (ALIMER_VERSION_MAJOR 0)
set (ALIMER_VERSION_MINOR 1)
set (ALIMER_VERSION_PATCH 0)
set (ALIMER_VERSION ${ALIMER_VERSION_MAJOR}.${ALIMER_VERSION_MINOR}.${ALIMER_VERSION_PATCH})
set (ALIMER_VERSION_ALIAS "WIP")

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

# Root and build dir.
set (ALIMER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (ALIMER_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source/ThirdParty)
mark_as_advanced(ALIMER_ROOT_DIR)
mark_as_advanced(ALIMER_THIRD_PARTY_DIR)

# Configure CMake global variables
if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
	set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

if (NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

if (NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
	set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

# Detect target platform
if( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
	set(ALIMER_PLATFORM_WINDOWS 1)
	set(ALIMER_PLATFORM_NAME "Win32")
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore" )
	set (WINDOWS_STORE ON CACHE BOOL "" FORCE)
	set(ALIMER_PLATFORM_UWP 1)
	set(ALIMER_PLATFORM_NAME "UWP")
	set(ALIMER_UWP_VERSION_MIN  "10.0.18362.0")
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
	if (IOS)
		set(ALIMER_PLATFORM_IOS 1)
		set(ALIMER_PLATFORM_NAME "iOS")
	else()
		set(ALIMER_PLATFORM_OSX 1)
		set(ALIMER_PLATFORM_NAME "macOS")
	endif()
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
	set (LINUX ON CACHE BOOL "" FORCE)
	set(ALIMER_PLATFORM_LINUX 1)
	set(ALIMER_PLATFORM_NAME "Linux")
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
	set(ALIMER_PLATFORM_ANDROID 1)
	set(ALIMER_PLATFORM_NAME "Android")
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten" )
	set(ALIMER_PLATFORM_WEB 1)
	set(ALIMER_PLATFORM_NAME "Web")
else()
	message(FATAL_ERROR "Unrecognized platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Define standard configurations
if( CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_CONFIGURATION_TYPES MATCHES "Debug;Dev;Release" )
	set(CMAKE_CONFIGURATION_TYPES "Debug;Dev;Release" CACHE STRING "List of supported configurations." FORCE)

    set(CMAKE_BUILD_TYPE Dev CACHE STRING "The default build type")
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})

    # Copy settings
    set(CMAKE_EXE_LINKER_FLAGS_DEV ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "" FORCE)
	set(CMAKE_SHARED_LINKER_FLAGS_DEV ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "" FORCE)
	set(CMAKE_STATIC_LINKER_FLAGS_DEV ${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "" FORCE)
	set(CMAKE_MODULE_LINKER_FLAGS_DEV ${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} CACHE STRING "" FORCE)
	set(CMAKE_CXX_FLAGS_DEV ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} CACHE STRING "" FORCE)
	set(CMAKE_C_FLAGS_DEV ${CMAKE_C_FLAGS_RELWITHDEBINFO} CACHE STRING "" FORCE)
	set(CMAKE_CSharp_FLAGS_DEV ${CMAKE_CSharp_FLAGS_RELWITHDEBINFO} CACHE STRING "" FORCE)
	set(CMAKE_RC_FLAGS_DEV ${CMAKE_RC_FLAGS_RELWITHDEBINFO} CACHE STRING "" FORCE)

	mark_as_advanced(FORCE CMAKE_EXE_LINKER_FLAGS_DEV)
	mark_as_advanced(FORCE CMAKE_SHARED_LINKER_FLAGS_DEV)
	mark_as_advanced(FORCE CMAKE_STATIC_LINKER_FLAGS_DEV)
	mark_as_advanced(FORCE CMAKE_MODULE_LINKER_FLAGS_DEV)
	mark_as_advanced(FORCE CMAKE_CXX_FLAGS_DEV)
	mark_as_advanced(FORCE CMAKE_C_FLAGS_DEV)
	mark_as_advanced(FORCE CMAKE_CSharp_FLAGS_DEV)
	mark_as_advanced(FORCE CMAKE_RC_FLAGS_DEV)
endif()

# Options
include(CMakeDependentOption)
option(ALIMER_BUILD_SHARED "Build engine as shared library" ON)
option(ALIMER_ENABLE_CODE_ANALYSIS "Use Static Code Analysis on build" OFF)
option(ALIMER_PROFILING "Enable performance profiling" ON)
option(ALIMER_NETWORK "Enable Networking system " ON)
option(ALIMER_AUDIO "Enable Audio system " ON)
option(ALIMER_PHYSICS "Enable Physics system" ON)
option(ALIMER_BUILD_ENABLE_CODE_ANALYSIS "Use Static Code Analysis on build" OFF)

if (IOS OR EMSCRIPTEN)
    set(ALIMER_BUILD_SHARED OFF CACHE BOOL "Always Disable shared library on (IOS, WEB)" FORCE)
elseif(ANDROID)
    set(ALIMER_BUILD_SHARED ON CACHE BOOL "Always enable shared library on (ANDROID)" FORCE)
endif ()

# RHI backends
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	option(ALIMER_RHI_D3D11 "RHI Direct3D11 backend" ON)
    option(ALIMER_RHI_D3D12 "RHI Direct3D12 backend" ON)
    option(ALIMER_RHI_GL "RHI OpenGL backend" ON)
    option(ALIMER_RHI_VULKAN "RHI Vulkan backend" ON)
    set(ALIMER_RHI_METAL OFF CACHE BOOL "Disable RHI Metal backend" FORCE)
elseif(APPLE)
    set(ALIMER_RHI_D3D11 OFF CACHE BOOL "Disable RHI D3D11 backend" FORCE)
    set(ALIMER_RHI_D3D12 OFF CACHE BOOL "Disable RHI D3D12 backend" FORCE)
    set(ALIMER_RHI_GL OFF CACHE BOOL "Disable RHI OpenGL backend" FORCE)
    set(ALIMER_RHI_VULKAN OFF CACHE BOOL "Disable RHI Vulkan backend" FORCE)
    option(ALIMER_RHI_METAL "RHI Metal backend" ON)
else()
	set(ALIMER_RHI_D3D11 OFF CACHE BOOL "Disable RHI D3D11 backend" FORCE)
    set(ALIMER_RHI_D3D12 OFF CACHE BOOL "Disable RHI D3D12 backend" FORCE)
	option(ALIMER_RHI_GL "RHI OpenGL backend" ON)
    option(ALIMER_RHI_VULKAN "RHI Vulkan backend" ON)
    set(ALIMER_RHI_METAL OFF CACHE BOOL "Disable RHI Metal backend" FORCE)
endif ()

if (ANDROID OR IOS OR EMSCRIPTEN)
	set(ALIMER_BUILD_EDITOR OFF CACHE INTERNAL "Disable Build tools" FORCE)
else()
	option(ALIMER_BUILD_EDITOR "Build tools and editors" ON)
endif()

# Install directories
set (INSTALL_BASE_INCLUDE_DIR include)
set (INSTALL_INCLUDE_DIR ${INSTALL_BASE_INCLUDE_DIR}/Alimer)
set (INSTALL_THIRDPARTY_DIR ${INSTALL_INCLUDE_DIR}/ThirdParty)

# Compiler-specific setup
if(MSVC)
  #set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:console /ENTRY:wWinMainCRTStartup")
  #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:windows /ENTRY:wWinMainCRTStartup")
endif ()

# Source
add_subdirectory(Source)

# Print current build configuration
message(STATUS "Alimer Engine version ${ALIMER_VERSION}")
message(STATUS "GIT Revision ID: ${ALIMER_REVISION_ID}")
message(STATUS "GIT Short ChangeSet: ${ALIMER_SHORT_CHANGE_SET}")
message(STATUS "GIT ChangeSet: ${ALIMER_CHANGE_SET}")
message(STATUS "GIT ChangeSet Date: ${ALIMER_CHANGE_SET_DATE}")
message(STATUS "Build platform: ${ALIMER_PLATFORM_NAME}")

if (ALIMER_BUILD_SHARED)
  message(STATUS "  Library         SHARED")
else ()
  message(STATUS "  Library         STATIC")
endif ()
message(STATUS "  Profiling       ${ALIMER_PROFILING}")
message(STATUS "  Network         ${ALIMER_NETWORK}")
message(STATUS "  Audio           ${ALIMER_AUDIO}")
message(STATUS "  Physics         ${ALIMER_PHYSICS}")

if (ALIMER_RHI_D3D11)
  message(STATUS "  RHI Backend:   Direct3D11 (ALIMER_RHI_D3D11)")
endif()

if (ALIMER_RHI_D3D12)
  message(STATUS "  RHI Backend:   Direct3D12 (ALIMER_RHI_D3D12)")
endif()

if (ALIMER_RHI_GL)
  message(STATUS "  RHI Backend:   OpenGL (ALIMER_RHI_GL)")
endif()

if (ALIMER_RHI_VULKAN)
  message(STATUS "  RHI Backend:   Vulkan (ALIMER_RHI_VULKAN)")
endif()

if (ALIMER_RHI_METAL)
  message(STATUS "  RHI Backend:   Metal (ALIMER_RHI_METAL)")
endif()

# Install files
# README.md and license
install (FILES
	${CMAKE_CURRENT_SOURCE_DIR}/README.md
	${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
	DESTINATION .
)
