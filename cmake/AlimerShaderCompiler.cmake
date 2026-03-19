# Modified version of: https://github.com/NVIDIAGameWorks/donut/blob/main/compileshaders.cmake

set (ALIMER_DEFAULT_VK_REGISTER_OFFSETS
    --bRegShift 0
    --tRegShift 1000
    --uRegShift 2000
    --sRegShift 3000
)

function(alimer_compile_shaders)
    set(options "")
    set(oneValueArgs TARGET CONFIG FOLDER DXIL DXBC SPIRV_DXC COMPILER_OPTIONS_DXIL COMPILER_OPTIONS_DXBC COMPILER_OPTIONS_SPIRV)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(params "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT params_TARGET)
        message(FATAL_ERROR "alimer_compile_shaders: TARGET argument missing")
    endif()
    if (NOT params_CONFIG)
        message(FATAL_ERROR "alimer_compile_shaders: CONFIG argument missing")
    endif()

    # just add the source files to the project as documents, they are built by the script
    set_source_files_properties(${params_SOURCES} PROPERTIES VS_TOOL_OVERRIDE "None") 

    add_custom_target(${params_TARGET}
        DEPENDS ShaderMake
        SOURCES ${params_SOURCES}
    )

    if (params_DXIL AND ALIMER_RHI_D3D12)
        set(compilerCommand ShaderMake
            --config ${params_CONFIG}
            --out ${params_DXIL}
            --platform DXIL
            --binaryBlob
            --WX
            --stripReflection
            -I ${ALIMER_SHADERS_DEFINITIONS_DIR}
            -I ${ALIMER_SHADERS_INCLUDE_DIR}
            --outputExt .bin
            --shaderModel 6_5
        )

        separate_arguments(params_COMPILER_OPTIONS_DXIL NATIVE_COMMAND "${params_COMPILER_OPTIONS_DXIL}")

        list(APPEND compilerCommand ${params_COMPILER_OPTIONS_DXIL})
        
        add_custom_command(TARGET ${params_TARGET} PRE_BUILD COMMAND ${compilerCommand})
    endif()

    if (params_DXBC AND ALIMER_RHI_D3D12 AND ALIMER_COMPILE_DXBC)
        set(compilerCommand ShaderMake
            --config ${params_CONFIG}
            --out ${params_DXBC}
            --platform DXBC
            --binaryBlob
            --WX
            --stripReflection
            -I ${ALIMER_SHADERS_INCLUDE_DIR}
            -D COMPILER_FXC
            --outputExt .bin
        )

        separate_arguments(params_COMPILER_OPTIONS_DXBC NATIVE_COMMAND "${params_COMPILER_OPTIONS_DXBC}")

        list(APPEND compilerCommand ${params_COMPILER_OPTIONS_DXBC})
        
        add_custom_command(TARGET ${params_TARGET} PRE_BUILD COMMAND ${compilerCommand})
    endif()

    if (params_SPIRV_DXC AND ALIMER_RHI_VULKAN)       
        set(compilerCommand ShaderMake
            --config ${params_CONFIG}
            --out ${params_SPIRV_DXC}
            --platform SPIRV
            --binaryBlob
            --stripReflection
            -I ${ALIMER_SHADERS_DEFINITIONS_DIR}
            -I ${ALIMER_SHADERS_INCLUDE_DIR}
            --WX
            --vulkanVersion 1.2
            --outputExt .bin
            ${ALIMER_DEFAULT_VK_REGISTER_OFFSETS}
        )

        separate_arguments(params_COMPILER_OPTIONS_SPIRV NATIVE_COMMAND "${params_COMPILER_OPTIONS_SPIRV}")

        list(APPEND compilerCommand ${params_COMPILER_OPTIONS_SPIRV})

        add_custom_command(TARGET ${params_TARGET} PRE_BUILD COMMAND ${compilerCommand})
    endif()

    if(params_FOLDER)
        set_target_properties(${params_TARGET} PROPERTIES FOLDER ${params_FOLDER})
    endif()
endfunction()


# Generates a build target that will compile shaders for a given config file for all enabled engine platforms.
#
# The shaders will be placed into subdirectories of ${OUTPUT_BASE}, with names compatible with
# the FindDirectoryWithShaderBin framework function.

function(alimer_compile_shaders_all_platforms)
    set(options "")
    set(oneValueArgs TARGET CONFIG FOLDER OUTPUT_BASE COMPILER_OPTIONS_DXIL COMPILER_OPTIONS_DXBC COMPILER_OPTIONS_SPIRV)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(params "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT params_TARGET)
        message(FATAL_ERROR "alimer_compile_shaders_all_platforms: TARGET argument missing")
    endif()
    if (NOT params_CONFIG)
        message(FATAL_ERROR "alimer_compile_shaders_all_platforms: CONFIG argument missing")
    endif()
    if (NOT params_OUTPUT_BASE)
        message(FATAL_ERROR "alimer_compile_shaders_all_platforms: OUTPUT_BASE argument missing")
    endif()

    alimer_compile_shaders(TARGET ${params_TARGET}
                          CONFIG ${params_CONFIG}
                          FOLDER ${params_FOLDER}
                          DXBC ${params_OUTPUT_BASE}/dxbc
                          DXIL ${params_OUTPUT_BASE}/dxil
                          SPIRV_DXC ${params_OUTPUT_BASE}/spirv
                          COMPILER_OPTIONS_DXIL ${params_COMPILER_OPTIONS_DXIL}
                          COMPILER_OPTIONS_DXBC ${params_COMPILER_OPTIONS_DXBC}
                          COMPILER_OPTIONS_SPIRV ${params_COMPILER_OPTIONS_SPIRV}
                          SOURCES ${params_SOURCES}
    )
endfunction()
