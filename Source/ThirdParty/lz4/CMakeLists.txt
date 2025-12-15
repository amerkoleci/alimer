# Copyright (c) Amer Koleci and Contributors.
# Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

file (GLOB_RECURSE SOURCE_FILES include/*.h src/*.c)
add_library (lz4 STATIC ${SOURCE_FILES})
#if (ALIMER_BUILD_SHARED)
#    target_compile_definitions(lz4 PRIVATE -DLZ4_DLL_EXPORT=1 PUBLIC -DLZ4_DLL_IMPORT=1)
#endif ()

target_include_directories(lz4
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

#install (DIRECTORY include/ DESTINATION ${DEST_THIRDPARTY_HEADERS_DIR}/ FILES_MATCHING PATTERN *.h)
