# ShaderMake

[![Build Status](https://github.com/NVIDIAGameWorks/ShaderMake/actions/workflows/build.yml/badge.svg)](https://github.com/NVIDIAGameWorks/ShaderMake/actions/workflows/build.yml)

ShaderMake is a frond-end tool for batch multi-threaded shader compilation developed by NVIDIA DevTech. It is compatible with Microsoft FXC and DXC compilers by calling them via API functions or executing them through command line.

Features:

- Generates DXBC, DXIL and SPIR-V code.
- Outputs results in 3 formats: native binary, header file, and a [binary blob](#user-content-shader-blob-api) containing all permutations for a given shader.
- Minimizes the number of re-compilation tasks by tracking file modification times and include trees.

During project deployment, the *CMake* script automatically searches for `fxc` and `dxc` and sets these variables:

- `FXC_PATH` - `fxc` from *Windows SDK*
- `DXC_PATH` - `dxc` from *Windows SDK*
- `DXC_SPIRV_PATH` - `dxc` with enabled SPIRV generation from *Vulkan SDK*

## Command line options

Usage:

```
ShaderMake.exe -p {DXBC|DXIL|SPIRV} --binary [--header --blob] -c "path/to/config"
        -o "path/to/output" --compiler "path/to/compiler" [other options]
        -D DEF1 -D DEF2=1 ... -I "path1" -I "path2" ...

    -h, --help                show this help message and exit
```

Required options:
- `-p, --platform=<str>` - DXBC, DXIL or SPIRV
- `-c, --config=<str>` - Configuration file with the list of shaders to compile
- `-o, --out=<str>` - Output directory
- `--binary` - Output native binary files
- `--header` - Output header files
- `--blob` - Output shader blob files
- `--compiler=<str>` - Path to a specific FXC/DXC compiler

Compiler settings:
- `-m, --shaderModel=<str>` - Shader model for DXIL/SPIRV (always SM 5.0 for DXBC)
- `-O, --optimization=<int>` - Optimization level 0-3 (default = 3, disabled = 0)
- `--WX` - Maps to '-WX' DXC/FXC option: warnings are errors
- `--allResourcesBound` - Maps to `-all_resources_bound` DXC/FXC option: all resources bound
- `--PDB` - Output PDB files in `out/PDB/` folder
- `--stripReflection` - Maps to `-Qstrip_reflect` DXC/FXC option: strip reflection information from a shader binary
- `--matrixRowMajor` - Maps to `-Zpr` DXC/FXC option: pack matrices in row-major order

Defines & include directories:
- `-I, --include=<str>` - Include directory(s)
- `-D, --define=<str>` - Macro definition(s) in forms 'M=value' or 'M'

Other options:
- `-f, --force` - Treat all source files as modified
- `--sourceDir=<str>` - Source code directory
- `--relaxedInclude=<str>` - Include file(s) not invoking re-compilation
- `--outputExt=<str>` - Extension for output files, default is one of `.dxbc`, `.dxil`, `.spirv`
- `--serial` - Disable multi-threading
- `--flatten` - Flatten source directory structure in the output directory
- `--continue` - Continue compilation if an error is occured
- `--useAPI` - Use *FXC (d3dcompiler)* or *DXC (dxcompiler)* API explicitly (Windows only)
- `--colorize` - Colorize console output
- `--verbose` - Print commands before they are executed

SPIRV options:
- `--vulkanVersion=<str>` - Vulkan environment version, maps to `-fspv-target-env` (default = 1.3)
- `--spirvExt=<str>` - Maps to `-fspv-extension` option: add SPIR-V extension permitted to use
- `--sRegShift=<int>` - SPIRV: register shift for sampler (`s#`) resources
- `--tRegShift=<int>` - SPIRV: register shift for texture (`t#`) resources
- `--bRegShift=<int>` - SPIRV: register shift for constant (`b#`) resources
- `--uRegShift=<int>` - SPIRV: register shift for UAV (`u#`) resources

## Config file structure

A config file consists of several lines, where each line has the following structure:

```
path/to/shader -T profile [-O3 -o "output/subdirectory" -E entry -D DEF1={0,1} -D DEF2={0,1,2} -D DEF3]
```

where:
- `path/to/shader` - shader source file
- `-T` - shader profile, can be:
  - `vs` - vertex
  - `ps` - pixel
  - `gs` - geometry
  - `hs` - hull
  - `ds` - domain
  - `cs` - compute
  - `ms` - mesh
  - `as` - amplification
- `-E` - (optional) entry point (`main` by default)
- `-D` - (optional) adds a macro definition to the list, optional range of possible values can be provided in `{}`
- `-O` - (optional) optimization level (global setting used by default)
- `-o` - (optional) output directory override

Additionally, the config file parser supports:

- One line comments starting with `//`
- `#ifdef D`, where `D` is a macro definition name (the statement resolves to `true` if `D` is defined in the command line)
- `#if 1` and `#if 0`
- `#else`
- `#endif`

## Shader blob API

When the `--blob` command line argument is specified, ShaderMake will package multiple permutations for the same shader into a single "blob" file. These files use a custom format that is somewhat similar to regular TAR. 

ShaderMake provides a small library with parsing functions to use these blob files in applications. This library can be statically linked with an application by including ShaderMake as a git submodule and linking the `ShaderMakeBlob` target to your application:

    target_link_libraries(my_target PRIVATE ShaderMakeBlob)

Then include `<ShaderMake/ShaderBlob.h>` and use the `ShaderMake::FindPermutationInBlob` to locate a specific shader version in a blob. If that is unsuccessful, the `ShaderMake::EnumeratePermutationsInBlob` and/or `ShaderMake::FormatShaderNotFoundMessage` functions can help you provide a helpful error message to the user.
