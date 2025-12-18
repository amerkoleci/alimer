// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Platform/AppPlatform.h"
#include "Alimer/Application.h"

using namespace Alimer;

void AppPlatform::OnReady()
{
    _app->InitBeforeRun();
}
