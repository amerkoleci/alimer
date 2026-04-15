// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Platform/Types.h"

#if defined(_WIN32)
typedef struct HINSTANCE__* HINSTANCE;
typedef HINSTANCE HMODULE;
#endif

namespace Alimer
{
#if defined(_WIN32)
    using NativeLibrary = HMODULE;
#else
    using NativeLibrary = void*;
#endif

    class ALIMER_API DynamicLibrary final
    {
    public:
        DynamicLibrary() = default;
        ~DynamicLibrary();

        DynamicLibrary(const DynamicLibrary&) = delete;
        DynamicLibrary& operator=(const DynamicLibrary&) = delete;

        DynamicLibrary(DynamicLibrary&& rhs);
        DynamicLibrary& operator=(DynamicLibrary&& rhs);

        // TODO: Use FileSystemPath
        bool Open(const std::string& libraryPath, std::string* error = nullptr);
        void Close();

        bool IsValid() const;

    private:
        NativeLibrary _handle = nullptr;
        bool _needsClose = false;
    };
}
