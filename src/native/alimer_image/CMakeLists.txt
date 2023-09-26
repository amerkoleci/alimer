# Define target name
set (TARGET_NAME alimer_image)

set(SOURCE_FILES
    alimer_image.h
    alimer_image.c
)

add_library(${TARGET_NAME} ${SOURCE_FILES})
target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_IMAGE_IMPLEMENTATION=1)
if (BUILD_SHARED_LIBS)
    target_compile_definitions (${TARGET_NAME} PRIVATE ALIMER_IMAGE_SHARED_LIBRARY=1)
endif ()

# Include directories
target_include_directories(${TARGET_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)

if(WIN32)
    target_compile_definitions(${TARGET_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif ()

#target_link_libraries(${TARGET_NAME} PRIVATE libktx)

install (FILES "alimer_image.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME})

install(TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
