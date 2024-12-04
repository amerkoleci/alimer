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

    if (EMSCRIPTEN)
        set_target_properties(${SAMPLE_NAME} PROPERTIES SUFFIX ".html")
    endif ()

    #if (WIN32)
    #    set_target_properties(${SAMPLE_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    #endif ()

    set_target_properties(${SAMPLE_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
        FOLDER "Samples"
    )
endfunction()

# Add samples
add_sample(01_HelloWorld)
