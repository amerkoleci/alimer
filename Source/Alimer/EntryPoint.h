// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Application.h"

#if defined(_WIN32) && !defined(ALIMER_WIN32_CONSOLE)
#include <Windows.h>
#ifdef _MSC_VER
#    include <crtdbg.h>
#endif
#endif

#if defined(_WIN32) && defined(_DEBUG) && !defined(ALIMER_WIN32_CONSOLE)
#define ALIMER_DEFINE_APPLICATION(class_name) \
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) \
{ \
    ALIMER_UNUSED(hInstance); \
    ALIMER_UNUSED(hPrevInstance); \
    ALIMER_UNUSED(lpCmdLine); \
    ALIMER_UNUSED(nCmdShow); \
    int crtDebugFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG); \
    crtDebugFlags |= _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF; \
    _CrtSetDbgFlag(crtDebugFlags); \
    class_name app;\
    app.Run(); \
    return 0; \
}
#elif defined(_WIN32) && !defined(ALIMER_WIN32_CONSOLE)
#define ALIMER_DEFINE_APPLICATION(class_name) \
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) \
{ \
    ALIMER_UNUSED(hInstance); \
    ALIMER_UNUSED(hPrevInstance); \
    ALIMER_UNUSED(lpCmdLine); \
    ALIMER_UNUSED(nCmdShow); \
    class_name app;\
    app.Run(); \
    return 0; \
}
#else
#define ALIMER_DEFINE_APPLICATION(class_name) \
int main(int argc, char** argv) \
{ \
    class_name app;\
    app.Run(); \
    return 0; \
}
#endif
