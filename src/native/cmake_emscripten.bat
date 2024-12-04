@echo off
emcmake cmake -S "." -B build_web -DCMAKE_INSTALL_PREFIX:String="SDK" %* && cmake --build build_web -j4
