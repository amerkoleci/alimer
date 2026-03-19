if (NOT ALIMER_SHADER_COMPILER)
    return ()
endif ()

file(GLOB engine_shaders
    "*.cfg"
    "*.hlsli"
	"*.hlsl"
	"${ALIMER_ROOT_DIR}/Source/Alimer/Shaders/*.hlsli"
	"${ALIMER_ROOT_DIR}/Source/Alimer/Shaders/*.h"
)

if(ALIMER_SHADERS_OUTPUT_DIR)
	set(output_base ${ALIMER_SHADERS_OUTPUT_DIR})
else()
	set(output_base ${CMAKE_CURRENT_BINARY_DIR})
endif()

alimer_compile_shaders_all_platforms(
	TARGET AlimerShaders
	CONFIG ${CMAKE_CURRENT_LIST_DIR}/Shaders.cfg
	OUTPUT_BASE ${output_base}
	SOURCES ${engine_shaders}
)
