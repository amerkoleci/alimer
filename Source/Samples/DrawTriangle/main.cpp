// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include <Alimer.h>
using namespace Alimer;

class HelloWorldApp final : public Application
{
private:
    

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
        return true;
    }

    void OnDraw(/*[[maybe_unused]] CommandBuffer* commandBuffer*/) override
    {
        
    }
};

ALIMER_DEFINE_APPLICATION(HelloWorldApp);
