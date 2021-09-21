// Copyright © Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include <Alimer.h>
#include "shaders/types.h"
using namespace Alimer;



class HelloWorldApp final : public Application
{
private:
    BufferRef vertexBuffer;
    BufferRef indexBuffer;
    TextureRef texture;

    PipelineRef renderPipeline;
    float rotationX = 0.0f;
    float rotationY = 0.0f;


public:
    HelloWorldApp()
    {
    }

    Settings SetupSettings() override
    {
        Settings settings;
        settings.title = "Hello World";
        //settings.graphicsApi = GraphicsAPI::Vulkan;
        return settings;
    }

    bool Initialize() override
    {
        const float vertices[] = {
            /* pos                  color                       uvs */
            -1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     0.0f, 0.0f,
             1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     1.0f, 0.0f,
             1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     1.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     0.0f, 1.0f,

            -1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 0.0f,
             1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     1.0f, 0.0f,
             1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     1.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 1.0f,

            -1.0f, -1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 0.0f,
            -1.0f,  1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     1.0f, 0.0f,
            -1.0f,  1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     1.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 1.0f,

             1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     0.0f, 0.0f,
             1.0f,  1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     1.0f, 0.0f,
             1.0f,  1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     1.0f, 1.0f,
             1.0f, -1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     0.0f, 1.0f,

            -1.0f, -1.0f, -1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     0.0f, 0.0f,
            -1.0f, -1.0f,  1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     1.0f, 0.0f,
             1.0f, -1.0f,  1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     1.0f, 1.0f,
             1.0f, -1.0f, -1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     0.0f, 1.0f,

            -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     0.0f, 0.0f,
            -1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     1.0f, 0.0f,
             1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     1.0f, 1.0f,
             1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     0.0f, 1.0f
        };
        const uint16_t indices[] = {
            0, 1, 2,  0, 2, 3,
            6, 5, 4,  7, 6, 4,
            8, 9, 10,  8, 10, 11,
            14, 13, 12,  15, 14, 12,
            16, 17, 18,  16, 18, 19,
            22, 21, 20,  23, 22, 20
        };
        vertexBuffer = Buffer::Create(sizeof(vertices), BufferUsage::Vertex, vertices, "VertexBuffer");
        indexBuffer = Buffer::Create(sizeof(indices), BufferUsage::Index, indices, "IndexBuffer");

        uint32_t pixels[4 * 4] = {
            0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000,
            0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
            0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000,
            0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
        };
        TextureData textureData;
        textureData.data = pixels;
        textureData.rowPitch = 4 * GetFormatBytesPerBlock(PixelFormat::RGBA8Unorm);
        texture = Texture::Create2D(4, 4, PixelFormat::RGBA8Unorm, 1, 1, TextureUsage::ShaderRead, &textureData);

        static const char* shaderSource = R"(
#include <shaders/common.hlsli>

struct VSInput 
{ 
    float3 Position : ATTRIBUTE0;
    float4 Color : ATTRIBUTE1;
    float2 TexCoord : ATTRIBUTE2;
};

struct PSInput 
{ 
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
};

PUSH_CONSTANT(PushDataConstants, constants);

ConstantBuffer<DrawData> draw : register(b0);
PSInput vertex_main(in VSInput input) 
{
    PSInput output;
    float4x4 worldViewProjection = draw.world; // mul(camera.viewProjection, draw.world);
    output.Position = mul(worldViewProjection, float4(input.Position, 1));
    output.Color    = input.Color;
    output.TexCoord = input.TexCoord;
    return output;
}

float4 pixel_main(in PSInput input) : SV_TARGET
{
    Texture2D DiffuseTexture = Texture2DTable[constants.dataIndex];
    float4 color = DiffuseTexture.Sample(LinearWrapSampler, input.TexCoord) * input.Color;
    return color;
}
)";

        ShaderRef vertexShader = Shader::Create(ShaderStages::Vertex, shaderSource, "vertex_main");
        ShaderRef pixelShader = Shader::Create(ShaderStages::Pixel, shaderSource, "pixel_main");

        RenderPipelineStateCreateInfo renderPipelineDesc;
        renderPipelineDesc.vertexShader = vertexShader;
        renderPipelineDesc.pixelShader = pixelShader;

        renderPipelineDesc.vertexLayout.attributes[0].format = VertexFormat::Float32x3;
        renderPipelineDesc.vertexLayout.attributes[1].format = VertexFormat::Float32x4;
        renderPipelineDesc.vertexLayout.attributes[2].format = VertexFormat::Float32x2;
        renderPipelineDesc.colorFormats[0] = PixelFormat::BGRA8UnormSrgb;// gGraphics().GetCurrentBackBuffer()->GetFormat();
        renderPipelineDesc.depthStencilFormat = PixelFormat::Undefined; // gGraphics().GetBackBufferDepthStencilTexture()->GetFormat();
        renderPipeline = Pipeline::Create(renderPipelineDesc);
        return true;
    }

    void OnDraw([[maybe_unused]] CommandBuffer& context) override
    {
        //time += (float)GetElapsedSeconds();
        rotationX += 0.01f;
        rotationY += 0.02f;

        DrawData drawData;
        drawData.world = Float4x4::Multiply(Float4x4::CreateRotationX(rotationX), Float4x4::CreateRotationY(rotationY));

        CameraData cameraData;

        // Bindless data
        PushDataConstants pushConstants{};
        pushConstants.dataIndex = texture->GetView()->GetBindlessSRV();

        auto size = GetWindow()->GetSize();
        cameraData.view = Float4x4::CreateLookAt(Vector3(0, 0, 5), Vector3::Zero, Vector3::UnitY);
        cameraData.projection = Float4x4::CreatePerspectiveFieldOfView(PiOver4, (float)size.x / size.y, 0.1f, 100.0f);
        cameraData.viewProjection = Float4x4::Multiply(cameraData.view, cameraData.projection);

        drawData.world = Float4x4::Multiply(drawData.world, cameraData.viewProjection);

        context.SetVertexBuffer(0, vertexBuffer.Get());
        context.SetIndexBuffer(indexBuffer.Get(), IndexType::UInt16, 0);
        context.BindPipeline(renderPipeline.Get());
        context.PushConstants(&pushConstants, sizeof(pushConstants));
        context.BindConstantBufferData(drawData, 0);
        
        //context.BindTexture(0, texture);
        context.DrawIndexed(36);
    }
};

ALIMER_DEFINE_APPLICATION(HelloWorldApp);
