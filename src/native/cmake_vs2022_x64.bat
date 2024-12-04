@echo off
cmake -S "." -B "build_win_x64" -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="win-x64-sdk" %*
echo Open build_win_x64\Alimer.sln to build the project.
