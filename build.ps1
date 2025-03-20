#!/usr/bin/env pwsh

Remove-Item -Recurse -Force build
New-Item -ItemType Directory -Force -Path build
Set-Location -Path build
cmake -S .. -B . -D CMAKE_CXX_FLAGS=-DUSE_BOUNDS_CHECKING \
    -G "Visual Studio 17 2022" -A x64
MsBuild.exe djinni.sln /t:Build /p:Configuration=Release
