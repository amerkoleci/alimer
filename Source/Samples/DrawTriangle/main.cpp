// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include <Alimer.h>
using namespace alimer;
using namespace alimer::rhi;

class HelloWorldApp final : public Application
{
private:
    TextureHandle texture;
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
        texture = rhiDevice->CreateTexture(TextureDesc::Tex2D(Format::RGBA8UNorm, 4, 4));
        texture->SetName("TEST");

        static const char* shaderSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};
PSInput vertex_main(in  uint    VertId : SV_VertexID) 
{
    float4 Pos[3];
    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
    Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);
    float3 Col[3];
    Col[0] = float3(1.0, 0.0, 0.0); // red
    Col[1] = float3(0.0, 1.0, 0.0); // green
    Col[2] = float3(0.0, 0.0, 1.0); // blue
    PSInput output;
    output.Pos   = Pos[VertId];
    output.Color = Col[VertId];
    return output;
}

float4 pixel_main(in PSInput input) : SV_TARGET
{
    float4 color = float4(input.Color.rgb, 1.0);
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
        commandList->SetPipeline(renderPipeline);
        commandList->Draw(0, 3);
    }
};

ALIMER_DEFINE_APPLICATION(HelloWorldApp);
