// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Platform/DynamicLibrary.h"

#if defined(_WIN32)
#include "Alimer/Platform/Win32/WindowsPlatform.h"
#elif defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#endif

using namespace Alimer;

DynamicLibrary::~DynamicLibrary()
{
    Close();
}

DynamicLibrary::DynamicLibrary(DynamicLibrary&& rhs)
{
    std::swap(_handle, rhs._handle);
    _needsClose = rhs._needsClose;
}

DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& rhs)
{
    std::swap(_handle, rhs._handle);
    _needsClose = rhs._needsClose;
    return *this;
}

bool DynamicLibrary::Open(const std::string& libraryPath, std::string* error)
{
#if defined(_WIN32)
    // Use SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS to avoid DLL search path attacks.
    const DWORD loadLibraryFlags =
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;
    _handle = LoadLibraryExA(libraryPath.c_str(), nullptr, /*(libraryPath.is_absolute()) ? LOAD_WITH_ALTERED_SEARCH_PATH : */loadLibraryFlags);
    if (_handle == nullptr && error != nullptr)
    {
        *error = "DynamicLibrary.Open: " + libraryPath + " Windows Error: " + std::to_string(GetLastError());
    }
#elif defined(__linux__) || defined(__APPLE__)
    mHandle = dlopen(libraryPath.c_str(), RTLD_NOW);

    if (mHandle == nullptr && error != nullptr) {
        *error = dlerror();
    }
#else
#error "Unsupported platform for DynamicLib"
#endif

    _needsClose = _handle != nullptr;
    return _needsClose;
}

void DynamicLibrary::Close()
{
    if (_handle == nullptr)
    {
        return;
    }

    // If the dynamic library was opened with OpenLoaded don't attempt to close it.
    if (_needsClose)
    {
#if defined(_WIN32)
#if defined(ALIMER_ENABLE_ASAN_UBSAN)
        // Freeing and reloading a DLL on Windows causes ASAN to detect ODR violations.
        // https://github.com/google/sanitizers/issues/89
        // In ASAN builds, we have to leak the DLL instead in case it gets loaded again later.
        FreeLibrary(static_cast<HMODULE>(_handle));
#endif
#elif defined(__linux__) || defined(__APPLE__)
        dlclose(_handle);
#else
#error "DynamicLibrary: Unsupported platform"
#endif
    }

    _needsClose = false;
    _handle = nullptr;
}

bool DynamicLibrary::IsValid() const
{
    return _handle != nullptr;
}
