@echo off
cmake -S "./../" -B "vs2026_win_x64" -G "Visual Studio 18 2026" -A x64 -DCMAKE_INSTALL_PREFIX:String=="win-x64-sdk" %*
echo Open vs2026_win_x64\Alimer.sln to build the project.
