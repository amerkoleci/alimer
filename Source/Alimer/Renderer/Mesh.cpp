// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Renderer/Mesh.h"

using namespace Alimer;

void Mesh::Register()
{
    RegisterFactory<Mesh>();
}

Mesh::Mesh(std::string_view name)
    : Asset(name)
{}

Mesh::~Mesh()
{
    ClearCpuData();
    Destroy();
}

void Mesh::Destroy()
{
}

void Mesh::ClearCpuData()
{
}
