// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Containers.h"
#include "Alimer/Math/Color.h"
#include "Alimer/Math/BoundingBox.h"
#include "Alimer/Renderer/Types.h"
#include "Alimer/Assets/Asset.h"
#include "Alimer/RHI/RHI.h"

namespace Alimer
{
    class ALIMER_API SubMesh final : public RefCounted
    {
        friend class Mesh;

    public:
        [[nodiscard]] Mesh* GetMesh() const { return _mesh; }
        [[nodiscard]] uint32_t GetIndexStart() const { return _indexStart; }
        [[nodiscard]] uint32_t GetIndexCount() const { return _indexCount; }
        [[nodiscard]] uint32_t GetMaterialIndex() const { return _materialIndex; }

    private:
        SubMesh(Mesh* mesh, uint32_t indexStart, uint32_t indexCount, uint32_t materialIndex = 0);

        Mesh* _mesh;
        uint32_t _indexStart;
        uint32_t _indexCount;
        uint32_t _materialIndex;
    };

    class ALIMER_API Mesh final : public Asset
    {
    public:
        static void Register();

        /// Constructor.
        explicit Mesh(StringView name = kEmptyStringView);

        /// Destructor.
        ~Mesh() override;

        void Destroy();

        void SetVertexCount(uint32_t vertexCount);
        void SetPositions(const Vector3* positions);
        void SetNormals(const Vector3* normals);
        void SetTangents(const Vector4* tangents);
        void SetTexCoords0(const Vector2* texcoords);
        void SetTexCoords1(const Vector2* texcoords);
        void SetColors(const Color* colors);
        void SetIndices(const uint16_t* indices, uint32_t indexCount);
        void SetIndices(const uint32_t* indices, uint32_t indexCount);

        [[nodiscard]] SubMesh* AddSubMesh(uint32_t indexStart, uint32_t indexCount, uint32_t materialIndex = 0);
        [[nodiscard]] uint32_t GetSubmeshCount() const { return (uint32_t)_subMeshes.size(); }
        [[nodiscard]] SubMesh* GetSubmesh(uint32_t index = 0) const { return _subMeshes[index].Get(); }

        [[nodiscard]] uint32_t GetVertexCount() const { return _vertexCount; }
        [[nodiscard]] uint32_t GetVertexStride() const { return _vertexStride; }

        /// Gets local-space bounding box.
        const BoundingBox& GetBoundingBox() const { return _boundingBox; }

        /// Set local-space bounding box.
        void SetBoundingBox(const BoundingBox& value);

        /// Recalculates local-space bounding box from vertex positions.
        void RecalculateBounds();

    private:
        void ClearCpuData();

        Vector<Vector3> _positions;
        Vector<Vector3> _normals;
        Vector<Vector4> _tangents;
        Vector<Vector2> _texCoords0;
        Vector<Vector2> _texCoords1;
        Vector<Color> _colors;
        Vector<UShort4> _jointIndices;
        Vector<Vector4> _jointWeights;
        Vector<uint32_t> _indices;

        uint32_t _vertexCount = 0;
        uint32_t _indexCount = 0;
        RHIIndexFormat _indexFormat = RHIIndexFormat::Uint16;
        uint64_t indexBufferOffset = 0;

        Vector<SharedPtr<SubMesh> > _subMeshes;
        BoundingBox _boundingBox;

        Vector<RHIVertexAttribute> _vertexAttributes;
        uint32_t _vertexStride = 0;
    };
}
