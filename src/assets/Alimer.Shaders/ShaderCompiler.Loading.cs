// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Reflection;
using System.Runtime.InteropServices;

namespace Alimer.Shaders;

partial class ShaderCompiler
{
    static ShaderCompiler()
    {
        NativeLibrary.SetDllImportResolver(Assembly.GetExecutingAssembly(), OnDllImport);
    }

    private static nint OnDllImport(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
    {
        if (libraryName.Equals("dxcompiler") && TryResolveLibrary(assembly, searchPath, out nint nativeLibrary))
        {
            return nativeLibrary;
        }

        return 0;
    }

    private static bool TryResolveLibrary(Assembly assembly, DllImportSearchPath? searchPath, out nint nativeLibrary)
    {
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            // Load DXIL first so that DXC doesn't fail to load it
            if (NativeLibrary.TryLoad("dxil.dll", out _) &&
                NativeLibrary.TryLoad("dxcompiler.dll", out nativeLibrary))
            {
                return true;
            }
        }
        else
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                // Load DXIL first so that DXC doesn't fail to load it
                if (NativeLibrary.TryLoad("libdxil.so", out _) &&
                    NativeLibrary.TryLoad("libdxcompiler.so", out nativeLibrary))
                {
                    return true;
                }
            }
        }

        nativeLibrary = 0;
        return false;
    }
}
