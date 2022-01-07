@echo off
set "InstallPath=C:/Users/Public/Documents/ghost"
set /p "InstallPath=Enter install path or press ENTER for default [%InstallPath%]: "
cmake -G "Visual Studio 17 2022" -A Win32 -B "build32" -D CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL
cmake --build build32 --config Release
cmake --install build32 --prefix %InstallPath%
cmake -G "Visual Studio 17 2022" -A x64 -B "build64" -D CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build build64 --config Release
cmake --install build64 --prefix %InstallPath%
