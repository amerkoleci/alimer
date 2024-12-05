# Groups sources into subfolders.d
macro(group_sources)
	file (GLOB_RECURSE children LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/**)
	foreach (child ${children})
		if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
		  string(REPLACE "/" "\\" groupname "${child}")
		  file (GLOB files LIST_DIRECTORIES false ${CMAKE_CURRENT_SOURCE_DIR}/${child}/*)
		  source_group(${groupname} FILES ${files})
		endif ()
	endforeach ()
endmacro()

# Setup library
set(TARGET_NAME alimer_native)
add_library(${TARGET_NAME} ${ALIMER_LIBRARY_TYPE}
	include/alimer_platform.h
    include/alimer.h
    include/alimer_gpu.h
    #include/alimer_audio.h
	src/alimer_internal.h
    src/alimer.cpp
    src/alimer_log.cpp
    src/alimer_image.cpp
    src/alimer_font.cpp
)

group_sources()

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
	target_compile_options(${TARGET_NAME} PRIVATE -Wall -Werror)
endif()

if(WIN32)
    target_compile_definitions(${TARGET_NAME} PRIVATE _UNICODE UNICODE _CRT_SECURE_NO_WARNINGS)
endif ()

target_link_libraries(${TARGET_NAME} PRIVATE
    stb
)

if (EMSCRIPTEN)
    target_sources(${TARGET_NAME} PRIVATE
        src/platforms/alimer_glfw.cpp
    )
    target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_PLATFORM_GLFW)

    set_target_properties(${TARGET_NAME} PROPERTIES ENABLE_EXPORTS ON)
    target_link_options(${TARGET_NAME} PUBLIC
        "-Os"
        "-sUSE_GLFW=3"
        "-sFORCE_FILESYSTEM=1"
    )

     # Handle WebGPU symbols
    target_link_options(${TARGET_NAME} PUBLIC -sUSE_WEBGPU)

    # Enable the use of emscripten_sleep()
	target_link_options(${TARGET_NAME} PUBLIC -sASYNCIFY)
elseif (ALIMER_USE_GLFW)
    target_sources(${TARGET_NAME} PRIVATE
        src/platforms/alimer_glfw.cpp
    )

    target_link_libraries(${TARGET_NAME} PRIVATE glfw)
    target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_PLATFORM_GLFW)
endif()

if (ALIMER_GPU)
    target_sources(${TARGET_NAME} PRIVATE
        src/alimer_gpu.cpp
        src/alimer_gpu_internal.h
    )
    target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_GPU)

    if (ALIMER_GPU_VULKAN)
        target_sources(${TARGET_NAME} PRIVATE
            src/gpu/alimer_gpu_vulkan_funcs.h
            src/gpu/alimer_gpu_vulkan.cpp
        )

        target_link_libraries(${TARGET_NAME} PRIVATE vma)
        
        target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_GPU_VULKAN)
    endif()

    if (ALIMER_GPU_D3D12)
        target_sources(${TARGET_NAME} PRIVATE
            src/gpu/alimer_gpu_d3d12.cpp
        )

        target_link_libraries(${TARGET_NAME} PRIVATE d3d12ma)
        
        target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_GPU_D3D12)
    endif()

    if (ALIMER_GPU_WEBGPU)
        target_sources(${TARGET_NAME} PRIVATE
            src/gpu/alimer_gpu_webgpu.cpp
        )
        target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_GPU_WEBGPU)
    endif()
endif ()

if (ALIMER_PHYSICS)
    target_sources(${TARGET_NAME} PRIVATE
        include/alimer_physics.h
        src/alimer_physics.cpp
    )

    target_link_libraries(${TARGET_NAME} PRIVATE Jolt)
    target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_PHYSICS)
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
