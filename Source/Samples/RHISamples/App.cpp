// Copyright (c) Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include "Sample.h"
#include "DrawTriangle.h"
#include "DrawIndexedQuad.h"
#include "DrawSpinningCube.h"
#include <Alimer/EntryPoint.h>

class RHISamplesApp final : public Application
{
public:
    ~RHISamplesApp() override;

    void Setup() override;
    void Initialize() override;
    void Draw([[maybe_unused]] RHICommandBuffer* commandBuffer, [[maybe_unused]] RHITexture* outputTexture) override;

private:
    Sample* _runningSample = nullptr;
};

RHISamplesApp::~RHISamplesApp()
{
    SafeDelete(_runningSample);
}

void RHISamplesApp::Setup()
{
    _options.name = "RHI Samples";
    //_options.graphics.preferredApi = GraphicsAPI::Vulkan;
    //settings.graphics.preferredApi = GraphicsAPI::WGPU;
#if defined(_DEBUG)
    //_options.graphics.validationMode = ValidationMode::Enabled;
#endif
}

void RHISamplesApp::Initialize()
{
    PixelFormat depthStencilFormat = PixelFormat::Depth32Float;

    //_runningSample = new DrawTriangle();
    //_runningSample = new DrawIndexedQuad();
    _runningSample = new DrawSpinningCube();

    _runningSample->Initialize(_rhiDevice,
        GetMainWindow()->GetSizeInPixels(),
        GetMainWindow()->GetColorFormat(),
        depthStencilFormat);
}

void RHISamplesApp::Draw([[maybe_unused]] RHICommandBuffer* commandBuffer, [[maybe_unused]] RHITexture* outputTexture)
{
    _runningSample->Draw(commandBuffer, outputTexture);
}


ALIMER_DEFINE_APPLICATION(RHISamplesApp);
