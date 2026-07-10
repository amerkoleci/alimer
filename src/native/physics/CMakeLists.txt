# Setup library
if (${ALIMER_PHYSICS} MATCHES "Jolt")
    set(SRC_FILE src/alimer_physics.cpp)
elseif (${ALIMER_PHYSICS} MATCHES "Box3D")
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
