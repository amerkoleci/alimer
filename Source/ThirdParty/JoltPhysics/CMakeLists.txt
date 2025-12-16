set(PHYSICS_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# When turning this option on, the library will be compiled using doubles for positions. This allows for much bigger worlds.
option(DOUBLE_PRECISION "Use double precision math" OFF)

# When turning this option on, the library will be compiled with debug symbols
option(GENERATE_DEBUG_SYMBOLS "Generate debug symbols" OFF)

# When turning this option on, the library will override the default CMAKE_CXX_FLAGS_DEBUG/RELEASE values, otherwise they will use the platform defaults
option(OVERRIDE_CXX_FLAGS "Override CMAKE_CXX_FLAGS_DEBUG/RELEASE" OFF)

# When turning this option on, the library will be compiled in such a way to attempt to keep the simulation deterministic across platforms
option(CROSS_PLATFORM_DETERMINISTIC "Cross platform deterministic" OFF)

# When turning this option on, the library will be compiled for ARM (aarch64-linux-gnu), requires compiling with clang
option(CROSS_COMPILE_ARM "Cross compile to aarch64-linux-gnu" OFF)

# When turning this option on, Jolt will be compiled as a shared library and public symbols will be exported.
option(BUILD_SHARED_LIBS "Compile Jolt as a shared library" OFF)

# When turning this option on, the library will be compiled with interprocedural optimizations enabled, also known as link-time optimizations or link-time code generation.
# Note that if you turn this on you need to use SET_INTERPROCEDURAL_OPTIMIZATION() or set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON) to enable LTO specificly for your own project as well.
# If you don't do this you may get an error: /usr/bin/ld: libJolt.a: error adding symbols: file format not recognized
option(INTERPROCEDURAL_OPTIMIZATION "Enable interprocedural optimizations" ON)

# When turning this on, in Debug and Release mode, the library will emit extra code to ensure that the 4th component of a 3-vector is kept the same as the 3rd component
# and will enable floating point exceptions during simulation to detect divisions by zero.
# Note that this currently only works using MSVC. Clang turns Float2 into a SIMD vector sometimes causing floating point exceptions (the option is ignored).
option(FLOATING_POINT_EXCEPTIONS_ENABLED "Enable floating point exceptions" ON)

# When turning this on, the library will be compiled with C++ exceptions enabled.
# This adds some overhead and Jolt doesn't use exceptions so by default it is off.
option(CPP_EXCEPTIONS_ENABLED "Enable C++ exceptions" OFF)

# When turning this on, the library will be compiled with C++ RTTI enabled.
# This adds some overhead and Jolt doesn't use RTTI so by default it is off.
option(CPP_RTTI_ENABLED "Enable C++ RTTI" OFF)

# Number of bits to use in ObjectLayer. Can be 16 or 32.
option(OBJECT_LAYER_BITS "Number of bits in ObjectLayer" 16)

# Select X86 processor features to use (if everything is off it will be SSE2 compatible)
option(USE_SSE4_1 "Enable SSE4.1" ON)
option(USE_SSE4_2 "Enable SSE4.2" ON)
option(USE_AVX "Enable AVX" ON)
option(USE_AVX2 "Enable AVX2" ON)
option(USE_AVX512 "Enable AVX512" OFF)
option(USE_LZCNT "Enable LZCNT" ON)
option(USE_TZCNT "Enable TZCNT" ON)
option(USE_F16C "Enable F16C" ON)
option(USE_FMADD "Enable FMADD" ON)

# Enable SIMD for the WASM build. Note that this is currently off by default since not all browsers support this.
# See: https://caniuse.com/?search=WebAssembly%20SIMD (Safari got support in March 2023 and was the last major browser to get support).
option(USE_WASM_SIMD "Enable SIMD for WASM" ${ALIMER_USE_WASM_SIMD})

# Enable all warnings
option(ENABLE_ALL_WARNINGS "Enable all warnings and warnings as errors" ON)

# Setting to periodically trace broadphase stats to help determine if the broadphase layer configuration is optimal
option(TRACK_BROADPHASE_STATS "Track Broadphase Stats" OFF)

# Setting to periodically trace narrowphase stats to help determine which collision queries could be optimized
option(TRACK_NARROWPHASE_STATS "Track Narrowphase Stats" OFF)

# Enable the debug renderer in the Debug and Release builds. Note that DEBUG_RENDERER_IN_DISTRIBUTION will override this setting.
option(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE "Enable debug renderer in Debug and Release builds" ON)

# Setting to enable the debug renderer in all builds.
# Note that enabling this reduces the performance of the library even if you're not drawing anything.
option(DEBUG_RENDERER_IN_DISTRIBUTION "Enable debug renderer in all builds" OFF)

# Enable the profiler in Debug and Release builds. Note that PROFILER_IN_DISTRIBUTION will override this setting.
option(PROFILER_IN_DEBUG_AND_RELEASE "Enable the profiler in Debug and Release builds" ON)

# Enable the profiler in all builds.
# Note that enabling this reduces the performance of the library.
option(PROFILER_IN_DISTRIBUTION "Enable the profiler in all builds" OFF)

# Setting this option will force the library to use malloc/free instead of allowing the user to override the memory allocator
option(DISABLE_CUSTOM_ALLOCATOR "Disable support for a custom memory allocator" OFF)

# Setting this option will force the library to use the STL vector instead of the custom Array class
option(USE_STD_VECTOR "Use std::vector instead of own Array class" OFF)

# Setting this option will compile the ObjectStream class and RTTI attribute information
option(ENABLE_OBJECT_STREAM "Compile the ObjectStream class and RTTI attribute information" ON)

# Enable installation
option(ENABLE_INSTALL "Generate installation target" OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/Jolt/Jolt.cmake)
if (XCODE)
	# Ensure that we enable SSE4.2 for the x86_64 build, XCode builds multiple architectures
	set_property(TARGET Jolt PROPERTY XCODE_ATTRIBUTE_OTHER_CPLUSPLUSFLAGS[arch=x86_64] "$(inherited) -msse4.2 -mpopcnt") 
endif()
