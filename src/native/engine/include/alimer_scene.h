// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_SCENE_H_
#define ALIMER_SCENE_H_ 1

#include "alimer.h"

/* Enums */

/* Structs */
typedef struct SceneMesh {
    char* name;
} SceneMesh;

typedef struct SceneMaterial {
    char* name;
} SceneMaterial;

typedef struct SceneNode {
    char* name;
} SceneNode;

typedef struct Scene {
    uint32_t meshCount;
    uint32_t materialCount;
    uint32_t nodeCount;

    SceneMesh* meshes;
    SceneMaterial* materials;
    SceneNode* nodes;
} Scene;

ALIMER_API Scene* alimerSceneCreateFromMemory(const void* pData, size_t dataSize);
ALIMER_API void alimerSceneDestroy(Scene* scene);

#endif /* ALIMER_SCENE_H_ */
