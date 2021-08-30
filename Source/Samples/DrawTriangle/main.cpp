// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include <Alimer.h>
using namespace alimer;
using namespace alimer::rhi;

class HelloWorldApp final : public Application
{
private:
    //TextureHandle texture;
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
        //texture = rhiDevice->CreateTexture(TextureDesc::Tex2D(Format::RGBA8UNorm, 4, 4));
        //texture->SetName("TEST");

        static const char* shaderSource = R"(
static const float2 g_positions[] = {
	float2(-0.5, -0.5),
	float2(0, 0.5),
	float2(0.5, -0.5)
};

static const float3 g_colors[] = {
	float3(1, 0, 0),
	float3(0, 1, 0),
	float3(0, 0, 1)	
};

struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};
PSInput vertex_main(in uint vertexId : SV_VertexID) 
{
    PSInput output;
    output.Pos   = float4(g_positions[vertexId], 0, 1);
    output.Color = g_colors[vertexId];
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
        renderPipelineDesc.colorFormats[0] = Format::BGRA8UNorm;
        renderPipelineDesc.depthStencilFormat = Format::Depth32Float;
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
