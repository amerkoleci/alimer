if (NOT ALIMER_SHADER_COMPILER)
    return ()
endif ()

file(GLOB engine_shaders
    "*.cfg"
    "*.hlsli"
	"*.hlsl"
)

alimer_compile_shaders_all_platforms(
	TARGET SamplesShaders
	CONFIG ${CMAKE_CURRENT_LIST_DIR}/Shaders.cfg
	OUTPUT_BASE ${ALIMER_SHADERS_OUTPUT_DIR}
	SOURCES ${engine_shaders}
    FOLDER "Samples"
)
