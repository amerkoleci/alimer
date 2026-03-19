set (SHADERMAKE_BIN_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin" CACHE STRING "")

project (ShaderMake LANGUAGES C CXX)

# ShaderMake executable
add_executable (ShaderMake
    src/argparse.c
    src/argparse.h
    src/ShaderMake.cpp 
)
target_compile_options (ShaderMake PRIVATE ${COMPILE_OPTIONS})
set_target_properties (ShaderMake PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${SHADERMAKE_BIN_OUTPUT_PATH}/$<CONFIG>")
set_property (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ShaderMake)
set_property (TARGET ShaderMake PROPERTY FOLDER ShaderMake)
target_link_libraries (ShaderMake PRIVATE ShaderMakeBlob dxc spirv_reflect)
target_link_libraries (ShaderMake PRIVATE DirectX-Headers)

if (MSVC)
    target_compile_definitions (ShaderMake PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX _CRT_SECURE_NO_WARNINGS)
    target_link_options (ShaderMake PRIVATE "/DELAYLOAD:dxcompiler.dll")
    target_link_libraries (ShaderMake PRIVATE d3dcompiler)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_link_libraries (ShaderMake PRIVATE pthread dl)
else ()
    target_link_libraries (ShaderMake PRIVATE stdc++fs pthread dl)
endif ()

if (WIN32)
    set (DXC_BIN_FOLDER "${ALIMER_ROOT_DIR}/Source/ThirdParty/dxc/bin/win-x64")
    set (DXIL_LIBRARY "dxil.dll")
    set (DXCOMPILER_LIBRARY "dxcompiler.dll")
elseif(UNIX)
    set (DXC_BIN_FOLDER "${ALIMER_ROOT_DIR}/Source/ThirdParty/dxc/bin/linux-x64")
    set (DXIL_LIBRARY "libdxil.so")
    set (DXCOMPILER_LIBRARY "libdxcompiler.so")
endif()

add_custom_command(
	TARGET ShaderMake POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DXC_BIN_FOLDER}/${DXIL_LIBRARY}" "$<TARGET_FILE_DIR:ShaderMake>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DXC_BIN_FOLDER}/${DXCOMPILER_LIBRARY}" "$<TARGET_FILE_DIR:ShaderMake>"
)
