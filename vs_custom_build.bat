@echo off
@rem -*- coding: cp932-dos; -*-
if not exist vcpkg\nul (
git clone https://github.com/microsoft/vcpkg.git
pushd vcpkg
call bootstrap-vcpkg.bat
vcpkg.exe install sdl2:x86-windows sdl2:x64-windows
popd
)

for %%d in (Debug Debug-MultiByte) do (
    if not exist %%d\nul mkdir %%d
    for %%f in (vcpkg\installed\x86-windows\debug\lib\SDL2d.lib vcpkg\installed\x86-windows\debug\bin\SDL2d.dll vcpkg\installed\x86-windows\debug\bin\SDL2d.pdb) do (
        if not exist %%d\%%~nxf copy %%f %%d
    )
)

for %%d in (Release Release-MultiByte) do (
    if not exist %%d\nul mkdir %%d
    for %%f in (vcpkg\installed\x86-windows\lib\SDL2.lib vcpkg\installed\x86-windows\bin\SDL2.dll vcpkg\installed\x86-windows\bin\SDL2.pdb) do (
        if not exist %%d\%%~nxf copy %%f %%d
    )
)
for %%d in (x64\Debug x64\Debug-MultiByte ) do (
    if not exist %%d\nul mkdir %%d
    for %%f in (vcpkg\installed\x64-windows\debug\lib\SDL2d.lib vcpkg\installed\x64-windows\debug\bin\SDL2d.dll vcpkg\installed\x64-windows\debug\bin\SDL2d.pdb) do (
        if not exist %%d\%%~nxf copy %%f %%d
    )
)

for %%d in (x64\Release x64\Release-MultiByte ) do (
    if not exist %%d\nul mkdir %%d
    for %%f in (vcpkg\installed\x64-windows\lib\SDL2.lib vcpkg\installed\x64-windows\bin\SDL2.dll vcpkg\installed\x64-windows\bin\SDL2.pdb) do (
        if not exist %%d\%%~nxf copy %%f %%d
    )
)
