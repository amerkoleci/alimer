// Copyright (c) Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include "DrawSpinningCube.h"
#include <array>

struct VertexPositionNormalTexture
{
    VertexPositionNormalTexture() = default;
    VertexPositionNormalTexture(const VertexPositionNormalTexture&) = default;
    VertexPositionNormalTexture& operator=(const VertexPositionNormalTexture&) = default;
    VertexPositionNormalTexture(VertexPositionNormalTexture&&) = default;
    VertexPositionNormalTexture& operator=(VertexPositionNormalTexture&&) = default;

    VertexPositionNormalTexture(const Vector3& position_, const Vector3& normal_, const Vector2& texcoord_) noexcept
        : position(position_)
        , normal(normal_)
        , texcoord(texcoord_)
    {}

    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
};

void DrawSpinningCube::Initialize(RHIDevice* device, const UInt2& windowSize, PixelFormat colorFormat, PixelFormat depthStencilFormat)
{
    Sample::Initialize(device, windowSize, colorFormat, depthStencilFormat);

    // A box has six faces, each one pointing in a different direction.
    constexpr uint32_t FaceCount = 6;

    static const Vector3 faceNormals[FaceCount] =
    {
        Vector3::UnitZ,
        Vector3::Forward,
        Vector3::UnitX,
        Vector3::Left,
        Vector3::UnitY,
        Vector3::Down,
    };

    static const Vector2 textureCoordinates[4] =
    {
        Vector2::UnitX,
        Vector2::One,
        Vector2::UnitY,
        Vector2::Zero,
    };

    Vector3 size(5.0f);
    Vector3 tsize = size / 2.0f;

    std::vector<VertexPositionNormalTexture> vertices;
    std::vector<uint16_t> indices;

    // Create each face in turn.
    for (uint32_t i = 0; i < FaceCount; i++)
    {
        Vector3 normal = faceNormals[i];

        // Get two vectors perpendicular both to the face normal and to each other.
        Vector3 basis = (i >= 4) ? Vector3::UnitZ : Vector3::UnitY;

        Vector3 side1 = Vector3::Cross(normal, basis);
        Vector3 side2 = Vector3::Cross(normal, side1);

        // Six indices (two triangles) per face.
        uint32_t vbase = (uint32_t)vertices.size();
        indices.push_back((uint16_t)vbase + 0);
        indices.push_back((uint16_t)vbase + 2);
        indices.push_back((uint16_t)vbase + 1);

        indices.push_back((uint16_t)vbase + 0);
        indices.push_back((uint16_t)vbase + 3);
        indices.push_back((uint16_t)vbase + 2);

        // Four vertices per face.
        // (normal - side1 - side2) * tsize // normal // t0
        vertices.push_back(VertexPositionNormalTexture((normal - side1 - side2) * tsize, normal, textureCoordinates[0]));

        // (normal - side1 + side2) * tsize // normal // t1
        vertices.push_back(VertexPositionNormalTexture(Vector3::Multiply(Vector3::Add(Vector3::Subtract(normal, side1), side2), tsize), normal, textureCoordinates[1]));

        // (normal + side1 + side2) * tsize // normal // t2
        vertices.push_back(VertexPositionNormalTexture(Vector3::Multiply(Vector3::Add(normal, Vector3::Add(side1, side2)), tsize), normal, textureCoordinates[2]));

        // (normal + side1 - side2) * tsize // normal // t3
        vertices.push_back(VertexPositionNormalTexture(Vector3::Multiply(Vector3::Subtract(Vector3::Add(normal, side1), side2), tsize), normal, textureCoordinates[3]));
    }

    _vertexBuffer = RHICreateBuffer(device, vertices, RHIBufferUsage::Vertex);
    _indexBuffer = RHICreateBuffer(device, indices, RHIBufferUsage::Index);

    ShaderModuleRef vertexShader = RHILoadShader(device, ShaderStages::Vertex, "Cube");
    ShaderModuleRef fragmentShader = RHILoadShader(device, ShaderStages::Fragment, "Cube");

    std::array<VertexAttribute, 3> vertexAttributes = {
        VertexAttribute{ VertexAttributeSemantic::Position, VertexAttributeFormat::Float32x3, offsetof(VertexPositionNormalTexture, position) },
        VertexAttribute{ VertexAttributeSemantic::Normal, VertexAttributeFormat::Float32x3, offsetof(VertexPositionNormalTexture, normal) },
        VertexAttribute{ VertexAttributeSemantic::TexCoord, VertexAttributeFormat::Float32x2, offsetof(VertexPositionNormalTexture, texcoord) }
    };

    VertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.attributeCount = static_cast<uint32_t>(vertexAttributes.size());
    vertexBufferLayout.attributes = vertexAttributes.data();

    RenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.label = "IndexedQuad";
    pipelineDesc.vertexBufferLayoutCount = 1u;
    pipelineDesc.vertexBufferLayouts = &vertexBufferLayout;
    pipelineDesc.vertexShader = vertexShader;
    pipelineDesc.fragmentShader = fragmentShader;
    pipelineDesc.colorAttachmentCount = 1u;
    pipelineDesc.colorAttachmentFormats = &colorFormat;
    pipelineDesc.depthStencilFormat = depthStencilFormat;
    _renderPipeline = device->CreateRenderPipeline(pipelineDesc);
}

void DrawSpinningCube::Draw(CommandBuffer* commandBuffer, RHITexture* outputTexture)
{
    static float rx = 0.0f;
    static float ry = 0.0f;

    rx += 1.0f;
    ry += 2.0f;

    const float aspect = static_cast<float>(outputTexture->GetWidth()) / outputTexture->GetHeight();

    Matrix4x4 rotX = Matrix4x4::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), rx);
    Matrix4x4 rotY = Matrix4x4::CreateFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), ry);
    Matrix4x4 worldMatrix = rotX * rotY;

    Matrix4x4 viewMatrix = Matrix4x4::CreateLookAt(Vector3(0, 5, 5), Vector3::Zero, Vector3::UnitY);
    Matrix4x4 projectionMatrix = Matrix4x4::CreatePerspectiveFieldOfView(ToRadians(60), aspect, 0.1f, 1000.0f);
    Matrix4x4 viewProjectionMatrix = viewMatrix * projectionMatrix;

    RenderPassColorAttachment colorAttachment;
    colorAttachment.view = outputTexture->GetDefaultView();
    colorAttachment.loadAction = LoadAction::Clear;
    colorAttachment.storeAction = StoreAction::Store;
    //colorAttachment.initialState = ResourceState::RenderTarget;
    //colorAttachment.finalState = ResourceState::CopySource;
    colorAttachment.clearColor = { { 0.3f, 0.3f, 0.3f, 1.0f } };

    RenderPassDepthStencilAttachment depthStencilAttachment;
    if (_depthStencilTexture)
    {
        depthStencilAttachment.view = _depthStencilTexture->GetDefaultView();
        depthStencilAttachment.depthClearValue = 1.0f;
        //depthStencilAttachment.depthClearValue = 0.0f; // Infinite reverse Z
    }

    RenderPassDesc renderPassDescriptor = {};
    renderPassDescriptor.colorAttachmentCount = 1u;
    renderPassDescriptor.colorAttachments = &colorAttachment;
    if (_depthStencilTexture)
    {
        renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;
    }

    RenderPassEncoder* renderPass = commandBuffer->BeginRenderPass(renderPassDescriptor);
    renderPass->SetPipeline(_renderPipeline.Get());
    renderPass->SetVertexBuffer(0, _vertexBuffer.Get());
    renderPass->SetIndexBuffer(_indexBuffer.Get(), 0, IndexFormat::Uint16);

    struct alignas(16) PushData
    {
        float4x4 worldMatrix;
        float4x4 viewProjectionMatrix;
    } pushData;
    pushData.worldMatrix = worldMatrix;
    pushData.viewProjectionMatrix = viewProjectionMatrix;

    renderPass->SetPushConstants(pushData);
    renderPass->DrawIndexed(36);
    renderPass->End();
}
