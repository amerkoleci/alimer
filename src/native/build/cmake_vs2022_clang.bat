@echo off
cmake -B "vs2022_clang" -S "./../" -G "Visual Studio 17 2022" -A x64 -T ClangCL -DCMAKE_INSTALL_PREFIX:String="SDK" %*
echo:
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo Make sure to install:
echo - C++ Clang Compiler for Windows 12.0.0+
echo - C++ Clang-cl for v143+ build tools (x64/x86)
echo Using the Visual Studio Installer
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo Open vs2022_clang\Alimer.sln to build the project.