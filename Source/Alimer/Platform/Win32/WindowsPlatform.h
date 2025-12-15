// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "AlimerConfig.h"
#include <winapifamily.h>

#ifndef UNICODE
#define UNICODE
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

// We don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// We dont' need <mcx.h>
#define NOMCX

// We dont' need <winsvc.h>
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

//#include <WinSock2.h>

#else

#define WINRT_LEAN_AND_MEAN

#endif

#include <Windows.h>
#ifdef SendMessage
#    undef SendMessage
#endif
#ifdef GetMessage
#    undef GetMessage
#endif
#ifdef GetObject
#    undef GetObject
#endif
#ifdef LoadLibrary
#    undef LoadLibrary
#endif

#include <string>
#include <string_view>
#include <memory>
#include <stdexcept>

namespace Alimer
{
    ALIMER_API std::string ToUtf8(std::wstring_view str);
    ALIMER_API std::wstring ToUtf16(std::string_view str);

    template <typename T>
    void SafeRelease(T& resource)
    {
        if (resource)
        {
            resource->Release();
            resource = nullptr;
        }
    }

    // Helper class for COM exceptions
    class COMException : public std::exception
    {
    public:
        COMException(HRESULT hr) noexcept : result(hr) {}

        const char* what() const noexcept override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw COMException(hr);
        }
    }

    inline std::string GetWin32ErrorString(DWORD errorCode)
    {
        char errorString[MAX_PATH];
        ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
            0,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errorString,
            MAX_PATH,
            NULL);

        return std::string(errorString);
    }

    inline std::wstring GetWin32ErrorStringWide(DWORD errorCode)
    {
        WCHAR errorString[MAX_PATH];
        ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
            0,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errorString,
            MAX_PATH,
            NULL);

        return std::wstring(errorString);
    }
}
