<h1 align="center" style="border-bottom: none;">
  <a href="https://github.com/amerkoleci/vortice">Vortice</a>
</h1>
<h3 align="center">Cross-platform 2D and 3D .NET 6.0 Game Engine written in modern C# 10.</h3>
<p align="center">
  <a href="#features">Features</a> |
  <a href="#building">Building 🔨</a> |
  <a href="#screenshots">Screenshots</a> |
  <a href="#dependencies">Dependencies</a> |
  <a href="#license">License</a>
<br/>
<br/>

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/amerkoleci/vortice/blob/main/LICENSE)
[![Build status](https://github.com/amerkoleci/vortice/workflows/ci/badge.svg)](https://github.com/amerkoleci/vortice/actions)
<a href="https://github.com/amerkoleci/vortice/issues"><img alt="Issues" src="https://img.shields.io/github/issues-raw/amerkoleci/vortice.svg?style=flat-square"/></a>
<a href="https://github.com/amerkoleci/vortice"><img alt="size" src="https://img.shields.io/github/repo-size/amerkoleci/vortice?style=flat-square"/></a>
<a href="https://github.com/amerkoleci/vortice/stargazers"><img alt="stars" src="https://img.shields.io/github/stars/amerkoleci/vortice?style=social"/></a>
<br/>
</p>

#

## Features

* Support for Windows, Linux, macOS.
* Modern rendering using Vulkan and Direct3D12.

## Building

```
git clone https://github.com/amerkoleci/vortice.git
```

To build with Release configuration
```
dotnet build Vortice.sln -c Release
```

To publish samples with Release configuration on windows
```
dotnet publish Vortice.sln -c Release -r win10-x64 
```

## Screenshots

## Dependencies

Uses the following open-source and third-party libraries:

- [terrafx.interop.windows](https://github.com/terrafx/terrafx.interop.windows): Interop bindings for Windows
- [terrafx.interop.d3d12memoryallocator](https://github.com/terrafx/terrafx.interop.d3d12memoryallocator): Interop bindings for D3D12MemoryAllocator
- [Vortice.Vulkan](https://github.com/amerkoleci/Vortice.Vulkan): Cross platform .NET bindings for Vulkan

## License

Licensed under the MIT license, see [LICENSE](https://github.com/amerkoleci/vortice/blob/main/LICENSE) for details.