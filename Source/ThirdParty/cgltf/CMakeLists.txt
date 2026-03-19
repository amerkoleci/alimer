# Copyright (c) Amer Koleci and Contributors.
# Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

add_library (cgltf STATIC
    cgltf.h
    cgltf_write.h
    cgltf.c
)

target_include_directories(cgltf
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
