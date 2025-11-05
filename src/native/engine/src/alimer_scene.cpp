// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"
#include "alimer_scene.h"

ALIMER_DISABLE_WARNINGS()
#define CGLTF_IMPLEMENTATION
//#define CGLTF_WRITE_IMPLEMENTATION
#include "third_party/cgltf.h"
//#include "third_party/cgltf_write.h"
ALIMER_ENABLE_WARNINGS()

static Scene* tryLoadGltfFromMemory(const void* pData, size_t dataSize)
{
    // Setup cgltf options
    cgltf_options options = {};

    cgltf_data* data = nullptr;
    cgltf_result result = cgltf_parse(&options, pData, (cgltf_size)dataSize, &data);
    if (result != cgltf_result_success)
    {
        return nullptr;
    }

#if defined(_DEBUG)
    // Validate the data
    result = cgltf_validate(data);
    if (result != cgltf_result_success)
    {
        // Continue anyway - validation failures might not be critical
    }
#endif

    Scene* scene = ALIMER_ALLOC(Scene);
    ALIMER_ASSERT(scene);

    if (data->materials_count > 0)
    {
        scene->materialCount = (uint32_t)data->materials_count;
        scene->materials = ALIMER_ALLOCN(SceneMaterial, scene->materialCount);

        for (cgltf_size i = 0; i < data->materials_count; ++i)
        {
            const cgltf_material* gltfMaterial = &data->materials[i];
        }
    }

    // Process scene nodes or all nodes if no scene is specified
    const cgltf_scene* gltf_scene = data->scene ? data->scene : (data->scenes_count > 0 ? &data->scenes[0] : nullptr);
    if (gltf_scene)
    {
        scene->nodeCount = (uint32_t)gltf_scene->nodes_count;
        scene->nodes = ALIMER_ALLOCN(SceneNode, scene->materialCount);

        for (cgltf_size i = 0; i < gltf_scene->nodes_count; ++i)
        {
            const cgltf_node* node = gltf_scene->nodes[i];
        }
    }
    else
    {
        // Process all nodes
        for (cgltf_size i = 0; i < data->nodes_count; ++i)
        {
            const cgltf_node* node = &data->nodes[i];
        }
    }

    cgltf_free(data);

    return scene;
}

Scene* alimerSceneCreateFromMemory(const void* pData, size_t dataSize)
{
    Scene* scene = nullptr;

    if ((scene = tryLoadGltfFromMemory(pData, dataSize)) != NULL)
        return scene;

    return scene;
}

void alimerSceneDestroy(Scene* scene)
{
    // TODO: Free scene data
    if (scene->meshCount)
    {
        alimerFree(scene->meshes);
    }

    if (scene->materialCount)
    {
        alimerFree(scene->materials);
    }

    if (scene->nodeCount)
    {
        alimerFree(scene->nodes);
    }

    alimerFree(scene);
}
