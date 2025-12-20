@echo off
cmake -S "./../" -B "vs2026_win_arm64" -G "Visual Studio 18 2026" -A ARM64 -DCMAKE_INSTALL_PREFIX:String=="win-arm64-sdk" %*
echo Open vs2026_win_arm64\Alimer.sln to build the project.
