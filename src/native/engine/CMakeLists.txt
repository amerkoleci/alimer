# Define target name
set (TARGET_NAME alimer_native)

add_library(${TARGET_NAME} ${ALIMER_LIBRARY_TYPE}
    # Public Headers
    include/alimer.h
    # Internal Headers
    src/alimer_platform.h
    src/alimer_internal.h
    # Source files
    src/alimer_platform.c
    src/alimer_log.c
    src/alimer_image.c
    src/alimer_font.c
    src/alimer_audio.c
)

target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_IMPLEMENTATION)
if (ALIMER_SHARED_LIBRARY)
    target_compile_definitions (${TARGET_NAME} PUBLIC ALIMER_SHARED_LIBRARY)
endif ()

target_include_directories(${TARGET_NAME}
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/third_party>
)

set_target_properties(
    ${TARGET_NAME} PROPERTIES

    # Postfix for different profiles
    DEBUG_POSTFIX "d"
    RELWITHDEBINFO_POSTFIX "rd"
    MINSIZEREL_POSTFIX "s"
)

if (MSVC)
    # Set warning level 3
    target_compile_options(${TARGET_NAME} PRIVATE /W3)

    # Warnings as Errors Enabled
    target_compile_options(${TARGET_NAME} PRIVATE /WX)

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

	# Enable function-level linking
	target_compile_options(${TARGET_NAME} PRIVATE /Gy)

    # Use fast floating point model
    target_compile_options(${TARGET_NAME} PRIVATE /fp:fast "$<$<NOT:$<CONFIG:DEBUG>>:/guard:cf>")
else()
    target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra)
endif()

if(WIN32)
    target_compile_definitions(${TARGET_NAME} PRIVATE _UNICODE UNICODE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(${TARGET_NAME} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif ()

if(ALIMER_USE_GLFW AND NOT (EMSCRIPTEN OR ANDROID OR WINDOWS_STORE))
    target_sources(${TARGET_NAME} PRIVATE
        src/alimer_platform_glfw.c
    )

    target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_USE_GLFW)
    target_link_libraries(${TARGET_NAME} PRIVATE glfw)
endif ()

if (ALIMER_VULKAN)
    target_sources(${TARGET_NAME} PRIVATE
        src/alimer_gpu_vulkan.cpp
    )

    target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_VULKAN)
endif ()

# Platform specific settings and libraries
if (WINDOWS_STORE)
elseif (WIN32)
else()
    target_link_libraries (${TARGET_NAME} PUBLIC dl)
    if (ANDROID)
        target_link_libraries (${TARGET_NAME} PUBLIC log android)
    elseif (EMSCRIPTEN)
    else()
        target_link_libraries (${TARGET_NAME} PUBLIC m pthread)
        if(NOT APPLE)
            target_link_libraries (${TARGET_NAME} PUBLIC rt uuid)
        endif ()
    endif ()
endif ()

if (ALIMER_INSTALL)
    install (FILES "include/alimer.h" DESTINATION "include")

    install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib"
    )
endif ()
