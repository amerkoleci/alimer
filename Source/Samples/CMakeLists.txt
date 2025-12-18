if (NOT ALIMER_SAMPLES)
    return ()
endif ()

function(add_sample SAMPLE_NAME)
    file(GLOB SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_NAME}/*.h"
	    "${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_NAME}/*.cpp"
    )

    if (ANDROID)
        add_library(${SAMPLE_NAME} SHARED ${SOURCE_FILES})
    else ()
    	add_executable(${SAMPLE_NAME} ${SOURCE_FILES})
    endif ()
    target_link_libraries(${SAMPLE_NAME} Alimer)

    if (EMSCRIPTEN)
        set_target_properties(${SAMPLE_NAME} PROPERTIES SUFFIX ".html")
    elseif (WIN32)
        set_target_properties(${SAMPLE_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    endif ()

    set_target_properties(${SAMPLE_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>"
        FOLDER "Samples"
    )

    # Copy Agility SDK dll
    if(ALIMER_RHI_D3D12 AND ALIMER_D3D12_AGILITY_SDK)
        add_custom_command(TARGET ${SAMPLE_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${SAMPLE_NAME}>/D3D12"
	        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${D3D12_CORE_DLL}" "$<TARGET_FILE_DIR:${SAMPLE_NAME}>/D3D12"
            COMMAND $<$<CONFIG:Debug>:${CMAKE_COMMAND}> -E copy_if_different "${D3D12_CORE_PDB}" "$<TARGET_FILE_DIR:${SAMPLE_NAME}>/D3D12"
            COMMAND $<$<CONFIG:Debug>:${CMAKE_COMMAND}> -E copy_if_different "${D3D12_SDK_LAYERS_DLL}" "$<TARGET_FILE_DIR:${SAMPLE_NAME}>/D3D12"
            COMMAND $<$<CONFIG:Debug>:${CMAKE_COMMAND}> -E copy_if_different "${D3D12_SDK_LAYERS_PDB}" "$<TARGET_FILE_DIR:${SAMPLE_NAME}>/D3D12"
        )
    endif ()

    if(ALIMER_INSTALL)
        install(
            TARGETS ${SAMPLE_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif()

    if (ALIMER_SHADER_COMPILER)
     #   add_dependencies(${SAMPLE_NAME} SamplesShaders)
    endif ()
endfunction()

# Add samples
add_sample(01_HelloWorld)
