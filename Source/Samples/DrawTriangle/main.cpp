// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include <Alimer.h>
using namespace alimer;
using namespace alimer::rhi;

class HelloWorldApp final : public Application
{
private:
    TextureHandle texture;

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
        return true;
    }

    void OnDraw([[maybe_unused]] rhi::ICommandList* commandList) override
    {
    }
};

ALIMER_DEFINE_APPLICATION(HelloWorldApp);
