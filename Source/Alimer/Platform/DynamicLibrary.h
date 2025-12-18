// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Platform/Types.h"
#include "Alimer/IO/Types.h"

namespace Alimer
{
    // Private implementation.
    struct DynamicLibraryImpl;

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
        void* _handle = nullptr;
        bool _needsClose = false;
    };
}
