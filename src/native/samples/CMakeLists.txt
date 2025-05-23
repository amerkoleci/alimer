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
        COMPILE_WARNING_AS_ERROR ON
    )

    if (MSVC)
        target_compile_options(${SAMPLE_NAME} PRIVATE /W4)
    else()
        target_compile_options(${SAMPLE_NAME} PRIVATE -Wall -Wextra -pedantic)
    endif()

    if (EMSCRIPTEN)
        set_target_properties(${SAMPLE_NAME} PROPERTIES SUFFIX ".html")
    endif ()

    if (WIN32)
        target_compile_definitions(${SAMPLE_NAME} PRIVATE _UNICODE UNICODE _CRT_SECURE_NO_WARNINGS)
    #    set_target_properties(${SAMPLE_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    endif ()

    set_target_properties(${SAMPLE_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        FOLDER "Samples"
    )
endfunction()

# Add samples
add_sample(01_HelloWorld)
