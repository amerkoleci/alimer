// Copyright (c) Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include <Alimer/Alimer.h>
#include <Alimer/EntryPoint.h>
using namespace Alimer;

class HelloWorldApp final : public Application
{
public:
    void Setup() override;
    void Initialize() override;
};

void HelloWorldApp::Setup()
{
}

void HelloWorldApp::Initialize()
{
}


ALIMER_DEFINE_APPLICATION(HelloWorldApp);
