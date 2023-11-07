@echo off
cmake -B "vs2022_arm64" -S "./../" -G "Visual Studio 17 2022" -A ARM64 -DCMAKE_INSTALL_PREFIX:String="SDK" %*
echo Open vs2022_arm64\Alimer.sln to build the project.