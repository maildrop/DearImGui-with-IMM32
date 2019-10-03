@echo off
@rem -*- coding: cp932-dos; -*-
if not exist vcpkg\nul (
git clone https://github.com/microsoft/vcpkg.git
pushd vcpkg
call bootstrap-vcpkg.bat
vcpkg.exe install sdl2:x86-windows sdl2:x64-windows
popd
)



