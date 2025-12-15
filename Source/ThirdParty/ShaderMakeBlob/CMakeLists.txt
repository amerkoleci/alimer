# Compile options
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
  set (SIMD -msse4.1)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set (COMPILE_OPTIONS ${SIMD} -Wextra)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set (COMPILE_OPTIONS ${SIMD} -Wextra)
elseif (MSVC)
    set (COMPILE_OPTIONS /W4 /WX /wd4324)
else ()
    message (WARNING "Unknown compiler!")
endif ()

# Library with blob read and write functions for use by client projects
add_library (ShaderMakeBlob STATIC
    include/ShaderMake/ShaderBlob.h
    src/ShaderBlob.cpp
)
target_include_directories (ShaderMakeBlob PUBLIC "include")
target_compile_options (ShaderMakeBlob PRIVATE ${COMPILE_OPTIONS})
set_target_properties (ShaderMakeBlob PROPERTIES 
    FOLDER ShaderMake
    POSITION_INDEPENDENT_CODE ON
)
