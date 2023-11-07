if (NOT ALIMER_SAMPLES)
    return ()
endif ()

function(add_sample SAMPLE_NAME)
    file(GLOB SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_NAME}/*.h"
	    "${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_NAME}/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_NAME}/*.cpp"
    )

    if (ANDROID)
        add_library(${SAMPLE_NAME} SHARED ${SOURCE_FILES})
    else ()
    	add_executable(${SAMPLE_NAME} ${SOURCE_FILES})
    endif ()

    target_link_libraries(${SAMPLE_NAME} alimer_native)

    set_target_properties(${SAMPLE_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        FOLDER "samples"
    )

    if(ALIMER_INSTALL)
        install(
            TARGETS ${SAMPLE_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif()
endfunction()

add_sample(hello_triangle)
