// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include <Alimer.h>
using namespace Alimer;
using namespace Alimer::rhi;

class HelloWorldApp final : public Application
{
private:
    BufferRef vertexBuffer;
    BufferRef indexBuffer;
    ShaderHandle vertexShader;
    ShaderHandle pixelShader;
    PipelineHandle renderPipeline;

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


        vertexShader = gGraphics().CreateShader(ShaderStages::Vertex, shaderSource, "vertex_main");
        pixelShader = gGraphics().CreateShader(ShaderStages::Pixel, shaderSource, "pixel_main");

        RenderPipelineDesc renderPipelineDesc;
        renderPipelineDesc.vertex = vertexShader;
        renderPipelineDesc.pixel = pixelShader;

        renderPipelineDesc.vertexLayout.attributes[0].format = VertexFormat::Float3;
        renderPipelineDesc.vertexLayout.attributes[1].format = VertexFormat::Float4;
        renderPipelineDesc.colorFormats[0] = gGraphics().GetCurrentBackBuffer()->GetFormat();
        renderPipelineDesc.depthStencilFormat = gGraphics().GetBackBufferDepthStencilTexture()->GetFormat();
        renderPipeline = gGraphics().CreateRenderPipeline(renderPipelineDesc);
        return true;
    }

    void OnDraw([[maybe_unused]] rhi::ICommandList* commandList) override
    {
        commandList->SetVertexBuffer(0, vertexBuffer);
        commandList->SetIndexBuffer(indexBuffer, 0, IndexType::UInt16);
        commandList->SetPipeline(renderPipeline);
        commandList->DrawIndexed(6);
    }
};

ALIMER_DEFINE_APPLICATION(HelloWorldApp);
