#!/usr/bin/env pwsh

Remove-Item -Recurse -Force build
New-Item -ItemType Directory -Force -Path build
Set-Location -Path build
cmake -S .. -B . -G "Visual Studio 17 2022"
MsBuild.exe djinni.sln /t:djinni_example /p:Platform="x64" /p:Configuration=Release
Copy-Item src\Release\djinni_example.exe -Destination ..\djinni_example.exe
Set-Location -Path ..
Remove-Item -Recurse -Force build
New-Item -ItemType Directory -Path djinni
Copy-Item src\djinni.h -Destination djinni\djinni.h
Copy-Item src\djinni -Destination djinni -Recurse
Compress-Archive -Path djinni -DestinationPath djinni.zip
Remove-Item -Recurse -Force djinni
Get-ChildItem djinni_example.exe