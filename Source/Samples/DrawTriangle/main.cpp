// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include <Alimer.h>
using namespace alimer;
using namespace alimer::rhi;

class HelloWorldApp final : public Application
{
private:
    BufferHandle vertexBuffer;
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
             0.0f, 0.5f, 0.5f,      1.0f, 0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
        };
        BufferDesc bufferDesc;
        bufferDesc.size = sizeof(vertices);
        bufferDesc.usage = BufferUsage::Vertex;
        vertexBuffer = rhiDevice->CreateBuffer(bufferDesc, vertices);

        static const char* shaderSource = R"(
struct VSInput 
{ 
    float3 Pos   : SV_POSITION; 
    float4 Color : COLOR; 
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

        vertexShader = rhiDevice->CreateShader(ShaderStages::Vertex, shaderSource, "vertex_main");
        pixelShader = rhiDevice->CreateShader(ShaderStages::Pixel, shaderSource, "pixel_main");

        RenderPipelineDesc renderPipelineDesc;
        renderPipelineDesc.vertex = vertexShader;
        renderPipelineDesc.pixel = pixelShader;
        renderPipeline = rhiDevice->CreateRenderPipeline(renderPipelineDesc);
        return true;
    }

    void OnDraw([[maybe_unused]] rhi::ICommandList* commandList) override
    {
        commandList->SetVertexBuffer(0, vertexBuffer);
        commandList->SetPipeline(renderPipeline);
        commandList->Draw(0, 3);
    }
};

ALIMER_DEFINE_APPLICATION(HelloWorldApp);
