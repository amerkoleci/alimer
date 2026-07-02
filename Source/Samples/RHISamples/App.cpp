// Copyright (c) Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include "Sample.h"
#include "DrawTriangle.h"
#include "DrawIndexedQuad.h"
#include "DrawSpinningCube.h"
#include "DrawTexturedCubeSample.h"
#include <Alimer/EntryPoint.h>

class RHISamplesApp final : public Application
{
public:
    ~RHISamplesApp() override;

    void Setup() override;
    void Initialize() override;
    void Draw([[maybe_unused]] RHICommandBuffer* commandBuffer, [[maybe_unused]] RHITexture* outputTexture) override;
    void OnWindowResized(Window* window) override;

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
    _options.graphics.preferredBackend = RHIBackend::Vulkan;
#if defined(_DEBUG)
    _options.graphics.validationMode = RHIValidationMode::Enabled;
#endif
}

void RHISamplesApp::Initialize()
{
    PixelFormat depthStencilFormat = PixelFormat::Depth32Float;

    //runningSample = new DrawTriangle();
    //runningSample = new DrawIndexedQuad();
    //_runningSample = new DrawSpinningCube();
    _runningSample = new DrawTexturedCubeSample();

    _runningSample->Initialize(_rhiDevice, GetMainWindow()->GetSizeInPixels(), GetMainWindow()->GetColorFormat(), depthStencilFormat);

    // Audio test
    //AudioClipRef clipFactory = CreateObject<AudioClip>();
    //AudioClipRef clip = GetAssets().Load<AudioClip>("Sounds/BGM.mp3");
    //AudioSource* source = new AudioSource(clip.Get());
    //source->Play();

    Vector3 position(145580.256f, 128.0f, -256.2f);

    SerializeValue value;
    value.PushFixedFloatArray(&position.x, 3);
    String jsonValue = value.ToJson();

    //SerializeValue readValue = SerializeValue::ParseJson(jsonValue);
    //ALIMER_ASSERT(readValue.IsArray());
    //ALIMER_ASSERT(readValue.Size() == 3);
    //float x = readValue[0].GetFloat();
    //float y = readValue[1].GetFloat();
    //float z = readValue[2].GetFloat();
}

void RHISamplesApp::Draw([[maybe_unused]] RHICommandBuffer* commandBuffer, [[maybe_unused]] RHITexture* outputTexture)
{
    _runningSample->Draw(commandBuffer, outputTexture);
}

void RHISamplesApp::OnWindowResized(Window* window)
{
    _runningSample->Resize(window->GetSizeInPixels());
}

ALIMER_DEFINE_APPLICATION(RHISamplesApp);
