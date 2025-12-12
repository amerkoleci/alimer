@echo off
cmake -S "." -B "build_win_x64_clang" -G "Visual Studio 18 2026" -A x64 -T ClangCL -DCMAKE_INSTALL_PREFIX:String="SDK" %*
echo:
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo Make sure to install:
echo - C++ Clang Compiler for Windows 12.0.0+
echo - C++ Clang-cl for v143+ build tools (x64/x86)
echo Using the Visual Studio Installer
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo Open build_win_x64_clang\Alimer.sln to build the project.