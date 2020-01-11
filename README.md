# DearImGui-with-IMM32
[Dear ImGui](https://github.com/ocornut/imgui) with IMM32

![screen](https://raw.githubusercontent.com/maildrop/DearImGui-with-IMM32/master/doc/imgui-on-the-spot.png?token=ACPJFWIO32M7UN2HFT4RVWS6EB4EQ)
```
git clone --recursive git@github.com:maildrop/DearImGui-with-IMM32.git
cd DearImGui-with-IMM32
vs_custom_build.bat
```
vs_custom_build.bat clones vcpkg and installs SDL2, so it will take some time.

and 

open file IMM32IMGUI.sln with Visual Studio 2019

### Software License
This software is the MIT License (MIT). (Excluding sample Japanese fonts)

### font license 
IMM32IMGUI/NotoSansMonoCJKjp-Regular.otf
```
Noto is a trademark of Google Inc. Noto fonts are open source. All Noto fonts are published under the SIL Open Font License, Version 1.1. Language data and some sample texts are from the Unicode CLDR project.
```
https://scripts.sil.org/cms/scripts/page.php?site_id=nrsi&id=OFL

## widgetTest project

Add [ImGuiColorTextEdit](https://github.com/BalazsJako/ImGuiColorTextEdit) to widgetTest and check the operation.
Unfortunately, I needed to add a bit of code to set the position of the IME Window. [Forked](https://github.com/maildrop/ImGuiColorTextEdit) for this. [maildrop/ImGuiColorTextEdit](https://github.com/maildrop/ImGuiColorTextEdit)

![TextEditor](https://raw.githubusercontent.com/maildrop/DearImGui-with-IMM32/master/doc/ImGui-TextEditor-IMM-Candidate-List.png)

# How to usage

## example 
- https://github.com/maildrop/DearImGui-with-IMM32/blob/master/IMM32IMGUI/main.cpp
- https://github.com/maildrop/DearImGui-with-IMM32/blob/master/widgetTest/widgetTest.cpp

## 1.include 
```
#if defined (_WIN32)
#include "imgui_imm32_onthespot.h"
#include "imgui_imm32_onthespot_sdl.h" /* If you are using SDL, include imgui_imm32_onthesport_sdl.h */
#endif /* defined( _WIN32 ) */
```

Include the necessary header files.


## 2.declare
```
ImGUIIMMCommunication imguiIMMCommunication{}; 
VERIFY( imguiIMMCommunication.subclassify( window ) );
```

Declare the function object at outside of the message loop.

DearImGui-with-IMM32 uses [SetWindowSubclass()](https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-setwindowsubclass) to get IMM32 window messages.

## 3.rendering
```
imguiIMMCommunication(); 
```
Finally, call the function object.

