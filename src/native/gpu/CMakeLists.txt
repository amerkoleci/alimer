# Setup library
set(TARGET_NAME alimer_gpu)
add_library(${TARGET_NAME} ${ALIMER_LIBRARY_TYPE}
	include/alimer_gpu.h
    src/alimer_gpu_internal.h
    src/alimer_gpu.cpp
    src/alimer_gpu_null.cpp
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
    target_compile_definitions(${TARGET_NAME} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
elseif(ANDROID)
    # Android specific 16 KB page sizes
    target_link_options(${TARGET_NAME} PRIVATE "-Wl,-z,max-page-size=16384")
endif ()

if (ALIMER_GPU_D3D12)
    target_sources(${TARGET_NAME} PRIVATE
        src/d3d12/alimer_gpu_d3d12.cpp
        src/d3d12/D3D12MemAlloc.cpp
    )

    target_link_libraries(${TARGET_NAME} PRIVATE DirectX-Headers)
    target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_GPU_D3D12)
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
