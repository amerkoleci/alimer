// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Renderer/Mesh.h"
#include "Alimer/Core/Log.h"

using namespace Alimer;

void Mesh::Register()
{
    RegisterFactory<Mesh>();
}

SubMesh::SubMesh(Mesh* mesh, uint32_t indexStart, uint32_t indexCount, uint32_t materialIndex)
    : _mesh(mesh), _indexStart(indexStart), _indexCount(indexCount), _materialIndex(materialIndex)
{
}

Mesh::Mesh(StringView name)
    : Asset(name)
{}

Mesh::~Mesh()
{
    ClearCpuData();
    Destroy();
}

void Mesh::SetVertexCount(uint32_t vertexCount)
{
    if (_vertexCount == vertexCount)
        return;

    _vertexCount = vertexCount;
    _positions.resize(vertexCount);
}

void Mesh::SetPositions(const Vector3* positions)
{
    if (!_vertexCount)
    {
        LOGF("Mesh::SetPositions: Vertex count is not set. Call SetVertexCount() before setting positions.");
        return;
    }

    memcpy(_positions.data(), positions, _vertexCount * sizeof(Vector3));
    _indexFormat = _positions.size() > 65536 ? RHIIndexFormat::Uint32 : RHIIndexFormat::Uint16;
}

void Mesh::SetNormals(const Vector3* normals)
{
    if (!_vertexCount)
    {
        LOGF("Mesh::SetNormals: Vertex count is not set. Call SetVertexCount() before setting normals.");
        return;
    }

    _normals.resize(_vertexCount);
    memcpy(_normals.data(), normals, _vertexCount * sizeof(Vector3));
}

void Mesh::SetTangents(const Vector4* tangents)
{
    if (!_vertexCount)
    {
        LOGF("Mesh::SetTangents: Vertex count is not set. Call SetVertexCount() before setting tangents.");
        return;
    }

    _tangents.resize(_vertexCount);
    memcpy(_tangents.data(), tangents, _vertexCount * sizeof(Vector4));
}

void Mesh::SetTexCoords0(const Vector2* texcoords)
{
    if (!_vertexCount)
    {
        LOGF("Mesh::SetTexCoords0: Vertex count is not set. Call SetVertexCount() before setting texcoords.");
        return;
    }

    _texCoords0.resize(_vertexCount);
    memcpy(_texCoords0.data(), texcoords, _vertexCount * sizeof(Vector2));
}

void Mesh::SetTexCoords1(const Vector2* texcoords)
{
    if (!_vertexCount)
    {
        LOGF("Mesh::SetTexCoords1: Vertex count is not set. Call SetVertexCount() before setting texcoords.");
        return;
    }

    _texCoords1.resize(_vertexCount);
    memcpy(_texCoords1.data(), texcoords, _vertexCount * sizeof(Vector2));
}

void Mesh::SetColors(const Color* colors)
{
    if (!_vertexCount)
    {
        LOGF("Mesh::SetColors: Vertex count is not set. Call SetVertexCount() before setting colors.");
        return;
    }

    _colors.resize(_vertexCount);
    memcpy(_colors.data(), colors, _vertexCount * sizeof(Color));
}

void Mesh::SetIndices(const uint16_t* indices_, uint32_t indexCount)
{
    _indexCount = indexCount;
    _indices.resize(indexCount);
    for (size_t i = 0; i < indexCount; ++i)
    {
        _indices[i] = (uint32_t)indices_[i];
    }

    _indexFormat = RHIIndexFormat::Uint16;
}

void Mesh::SetIndices(const uint32_t* indices_, uint32_t indexCount)
{
    _indexCount = indexCount;
    _indices.resize(indexCount);
    memcpy(_indices.data(), indices_, indexCount * sizeof(uint32_t));
    _indexFormat = RHIIndexFormat::Uint32;
}

SubMesh* Mesh::AddSubMesh(uint32_t indexStart, uint32_t indexCount, uint32_t materialIndex)
{
    SharedPtr<SubMesh> subMesh(new SubMesh(this, indexStart, indexCount, materialIndex));
    _subMeshes.push_back(subMesh);
    return subMesh.Get();
}


void Mesh::SetBoundingBox(const BoundingBox& value)
{
    _boundingBox = value;
}

void Mesh::RecalculateBounds()
{
    _boundingBox = BoundingBox::CreateFromPoints(_positions.data(), _positions.size());
}

void Mesh::Destroy()
{
}

void Mesh::ClearCpuData()
{
}
