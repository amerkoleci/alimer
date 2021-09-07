// Copyright (c) Amer Koleci.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include <Alimer.h>
using namespace Alimer;

class HelloWorldApp final : public Application
{
private:
    BufferRef vertexBuffer;
    BufferRef indexBuffer;
    PipelineRef renderPipeline;

public:
    HelloWorldApp()
    {
    }

    Settings SetupSettings() override
    {
        Settings settings;
        settings.title = "Hello World";
        return settings;
    }

    bool Initialize(int argc, const char* argv[]) override
    {
        const float vertices[] = {
            /* positions            colors */
            -0.5f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
             0.5f,  0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
             0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f,
            -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 0.0f, 1.0f,
        };
        const uint16_t indices[] = {
            0, 1, 2,    /* first triangle */
            0, 2, 3,    /* second triangle */
        };
        vertexBuffer = Buffer::Create(vertices, sizeof(vertices), BufferUsage::Vertex, "VertexBuffer");
        indexBuffer = Buffer::Create(indices, sizeof(indices), BufferUsage::Index);

        SamplerDesc samplerDesc;
        auto test = Sampler::Create(samplerDesc);

        static const char* shaderSource = R"(
struct VSInput 
{ 
    float3 Pos   : ATTRIBUTE0; 
    float4 Color : ATTRIBUTE1; 
};

struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float4 Color : COLOR; 
};
PSInput vertex_main(in VSInput input) 
{
    PSInput output;
    output.Pos   = float4(input.Pos, 1);
    output.Color = input.Color;
    return output;
}

float4 pixel_main(in PSInput input) : SV_TARGET
{
    float4 color = input.Color;
    return color;
}
)";

        ShaderRef vertexShader = Shader::Create(ShaderStages::Vertex, shaderSource, "vertex_main");
        ShaderRef pixelShader = Shader::Create(ShaderStages::Pixel, shaderSource, "pixel_main");

        RenderPipelineDesc renderPipelineDesc;
        renderPipelineDesc.vertex = vertexShader;
        renderPipelineDesc.pixel = pixelShader;

        renderPipelineDesc.vertexLayout.attributes[0].format = VertexFormat::Float3;
        renderPipelineDesc.vertexLayout.attributes[1].format = VertexFormat::Float4;
        renderPipelineDesc.colorFormats[0] = gGraphics().GetCurrentBackBuffer()->GetFormat();
        renderPipelineDesc.depthStencilFormat = gGraphics().GetBackBufferDepthStencilTexture()->GetFormat();
        renderPipeline = Pipeline::Create(renderPipelineDesc);
        return true;
    }

    void OnDraw([[maybe_unused]] CommandBuffer* commandBuffer) override
    {
        commandBuffer->SetVertexBuffer(0, vertexBuffer.Get());
        commandBuffer->SetIndexBuffer(indexBuffer.Get(), 0, IndexType::UInt16);
        commandBuffer->SetPipeline(renderPipeline.Get());
        commandBuffer->DrawIndexed(6);
    }
};

ALIMER_DEFINE_APPLICATION(HelloWorldApp);
