// Copyright © Amer Koleci and Contributors.
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

    private static IntPtr OnDllImport(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
    {
        if (libraryName is not "dxcompiler")
        {
            return IntPtr.Zero;
        }

        string rid = RuntimeInformation.ProcessArchitecture switch
        {
            Architecture.X64 => "win-x64",
            Architecture.Arm64 => "win-arm64",
            _ => default(NotSupportedException).Throw<string>()
        };

        // Test whether the native libraries are present in the same folder of the executable
        // (which is the case when the program was built with a runtime identifier), or whether
        // they are in the "runtimes\win-x64\native" folder in the executable directory.
        string nugetNativeLibsPath = Path.Combine(AppContext.BaseDirectory, $@"runtimes\{rid}\native");
        bool isNuGetRuntimeLibrariesDirectoryPresent = Directory.Exists(nugetNativeLibsPath);

        if (isNuGetRuntimeLibrariesDirectoryPresent)
        {
            string dxcompilerPath = Path.Combine(AppContext.BaseDirectory, $@"runtimes\{rid}\native\dxcompiler.dll");
            string dxilPath = Path.Combine(AppContext.BaseDirectory, $@"runtimes\{rid}\native\dxil.dll");

            // Load DXIL first so that DXC doesn't fail to load it, and then DXIL, both from the NuGet path
            if (NativeLibrary.TryLoad(dxilPath, out _) &&
                NativeLibrary.TryLoad(dxcompilerPath, out IntPtr handle))
            {
                return handle;
            }
        }
        else
        {
            // Even when the two libraries are correctly copied next to the executable in use, we load them
            // manually to ensure the operation is successful. This is to avoid failures in cases such as when
            // doing "dotnet bin\MyApp.dll", ie. when the host is in another path than the executable in use.
            // This is probably because DXIL is a native dependency for DXC, but the way Windows loads these
            // libraries doesn't take into account the .NET concepts of "app directory": neither the current "bin"
            // directory nor the "process directory", which is "C:\Program Files\dotnet", actually contain the
            // native library we need, hence the runtime crash. Manually loading the library this way solves this.
            if (NativeLibrary.TryLoad("dxil", assembly, searchPath, out _) &&
                NativeLibrary.TryLoad("dxcompiler", assembly, searchPath, out IntPtr handle))
            {
                return handle;
            }
        }

        return IntPtr.Zero;
    }
}
