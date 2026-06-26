// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/Scene/Scene.h"
#include "Alimer/Scene/Entity.h"
//#include "Alimer/Scene/Prefab.h"
#include "Alimer/Physics/Physics.h"
#include "Alimer/Core/JobSystem.h"
#include <cgltf.h>

using namespace Alimer;

void Scene::Register()
{
    static bool registered = false;
    if (registered)
        return;
    registered = true;

    RegisterFactory<Scene>();
    Entity::Register();
}

Scene::Scene(StringView name)
    : _name(name)
{
    //_physicsWorld = Physics::GetBackend()->CreatePhysicsWorld();
}

Scene::~Scene()
{
    _physicsWorld.Reset();
}

void Scene::SetName(const String& name)
{
    _name = name;
}

void Scene::SetName(StringView name)
{
    _name = name;
}

bool Scene::LoadGLTF(const String& path)
{
    cgltf_options options = {};
    cgltf_data* gltf = nullptr;

    if (cgltf_parse_file(&options, path.c_str(), &gltf) != cgltf_result_success)
    {
        LOGE("Scene::LoadGLTF: failed to parse '{}'", path.c_str());
        return false;
    }

    if (cgltf_load_buffers(&options, gltf, path.c_str()) != cgltf_result_success)
    {
        LOGE("Scene::LoadGLTF: failed to load buffers for '{}'", path.c_str());
        cgltf_free(gltf);
        return false;
    }

    // Materials
    for (int i = 0; i < gltf->materials_count; ++i)
    {
        cgltf_material* gltf_material = gltf->materials + i;
    }

    auto processNode = [&](auto& self, const cgltf_node* node) -> void
        {
            if (node->mesh)
            {
            }

            for (cgltf_size ci = 0; ci < node->children_count; ++ci)
                self(self, node->children[ci]);
        };

    // Walk all scenes
    for (cgltf_size si = 0; si < gltf->scenes_count; ++si)
    {
        const cgltf_scene& scene = gltf->scenes[si];
        for (cgltf_size ni = 0; ni < scene.nodes_count; ++ni)
            processNode(processNode, scene.nodes[ni]);
    }

    cgltf_free(gltf);

    return true;
}
