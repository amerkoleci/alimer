if (NOT ALIMER_PHYSICS)
    return ()
endif ()

# Setup library
if (${ALIMER_PHYSICS} MATCHES "Jolt")
    # Add Jolt dependency
    CPMAddPackage(
        NAME JoltPhysics
        GITHUB_REPOSITORY jrouwe/JoltPhysics
        GIT_TAG "v5.6.0"
        SOURCE_SUBDIR Build
        OPTIONS
            "DOUBLE_PRECISION OFF"
            "JPH_BUILD_SHARED_LIBS OFF"
            "GENERATE_DEBUG_SYMBOLS OFF"            # Default: ON
            "ENABLE_INSTALL OFF"                    # Default: ON
            "ENABLE_ALL_WARNINGS OFF"               # Default: ON
            "OBJECT_LAYER_BITS 32"                  # Default: 16 
            "ENABLE_OBJECT_STREAM OFF"              # Default: ON
            # Option to compile with DirectX 12 compute
            "JPH_USE_DX12 ON"
            # Option to compile with Vulkan compute
            "JPH_USE_VK ON"
            # Option to compile with Metal compute
            "JPH_USE_MTL ON"
            # Option to compile with CPU fallback compute
            "JPH_USE_CPU_COMPUTE ON"
            # Option to enable shader debug symbols
            "JPH_SHADER_DEBUG_SYMBOLS OFF"
            # Option to enable shader optimizations. When this is on, the shaders will be optimized for performance, otherwise they're suitable for debugging.
            "JPH_SHADER_OPTIMIZATION ON"
    )
    set_property(TARGET Jolt PROPERTY FOLDER "ThirdParty")

    set(SRC_FILE src/alimer_physics_jolt.cpp)
elseif (${ALIMER_PHYSICS} MATCHES "Box3D")
    # Add Box3D dependency
    CPMAddPackage(
        NAME box3d
        GITHUB_REPOSITORY erincatto/box3d
        GIT_TAG main
        OPTIONS
            "BOX3D_DOUBLE_PRECISION OFF"
    )
    set_property(TARGET box3d PROPERTY FOLDER "ThirdParty")
    set(SRC_FILE src/alimer_physics_box3d.cpp)
else()
    message(FATAL_ERROR "Unknown physics engine: ${ALIMER_PHYSICS}")
endif()

set(TARGET_NAME alimer_physics)
add_library(${TARGET_NAME} ${ALIMER_LIBRARY_TYPE}
	include/alimer_physics.h
    ${SRC_FILE}
)

target_include_directories(${TARGET_NAME}
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_IMPLEMENTATION)
if (ALIMER_BUILD_SHARED)
    target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_SHARED_LIBRARY)
endif ()

set_target_properties(${TARGET_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

# Setup MSVC/Windows Flags
if (MSVC)
    #target_compile_options(${TARGET_NAME} PRIVATE /W4)
    target_compile_options(${TARGET_NAME} PRIVATE /W3)

    # Warnings as Errors Enabled
    target_compile_options(${TARGET_NAME} PRIVATE /WX)
else()
	target_compile_options(${TARGET_NAME} PRIVATE -Wall)

    # Use fast floating point model
    #target_compile_options(${TARGET_NAME} PRIVATE -ffast-math -fno-finite-math-only -ffp-contract=fast)
endif()

if(WIN32)
    target_compile_definitions(${TARGET_NAME} PRIVATE _UNICODE UNICODE _CRT_SECURE_NO_WARNINGS)
elseif(ANDROID)
    # Android specific 16 KB page sizes
    target_link_options(${TARGET_NAME} PRIVATE "-Wl,-z,max-page-size=16384")
endif ()

if (${ALIMER_PHYSICS} MATCHES "Jolt")
    target_link_libraries(${TARGET_NAME} PRIVATE Jolt)
elseif (${ALIMER_PHYSICS} MATCHES "Box3D")
    target_link_libraries(${TARGET_NAME} PRIVATE box3d)
endif()


# Install
if (ALIMER_INSTALL)
    install (
        DIRECTORY include
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}
        FILES_MATCHING PATTERN *.h
    )

    install(TARGETS ${TARGET_NAME}
        ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION  ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
    )
endif()
