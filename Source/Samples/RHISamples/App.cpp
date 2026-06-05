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
    //_options.graphics.preferredBackend = RHIBackend::Vulkan;
#if defined(_DEBUG)
    _options.graphics.validationMode = RHIValidationMode::Enabled;
#endif
}

void RHISamplesApp::Initialize()
{
    PixelFormat depthStencilFormat = PixelFormat::Depth32Float;

    _runningSample = new DrawTriangle();
    //_runningSample = new DrawIndexedQuad();
    //_runningSample = new DrawSpinningCube();

    _runningSample->Initialize(_rhiDevice,
        GetMainWindow()->GetSizeInPixels(),
        GetMainWindow()->GetColorFormat(),
        depthStencilFormat);

    // Audio test
    //AudioClipRef clipFactory = CreateObject<AudioClip>();
    //AudioClipRef clip = GetAssets().Load<AudioClip>("Sounds/BGM.mp3");
    //AudioSource* source = new AudioSource(clip.Get());
    //source->Play();

    {
        JsonSerializer serializer;
        float x = 3.0f;
        float y = -4.55f;
        Vector2 vec2{ 1.0f, 2.0f };
        std::string hello = "Hello";
        serializer.BeginObject("TestArray", true);
        serializer.Serialize(nullptr, x);
        serializer.Serialize(nullptr, y);
        //serializer.Serialize("Hello", "World");
        serializer.Serialize(nullptr, hello);
        serializer.Serialize(nullptr, vec2);
        serializer.EndObject();
#if 0
        std::string str = serializer.ToString();
        LOGI("Serialized JSON: {}", str);

        JsonSerializer deserializer(str);
        deserializer.BeginObject("TestArray", true);
        deserializer.Serialize(nullptr, x);
        deserializer.Serialize(nullptr, y);
        //serializer.Serialize("Hello", "World");
        //deserializer.Serialize(nullptr, hello);
        //deserializer.Serialize(nullptr, vec2);
        deserializer.EndObject();

        //deserializer.BeginObject("TestArrayNone", false);
       // deserializer.EndObject();  
#endif // 0

    }
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
