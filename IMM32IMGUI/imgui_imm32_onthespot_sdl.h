#pragma once

#if !defined(IMGUI_IMM32_ONTHESPOT_SDL_H_HEADER_GUARD_7058bc82_a46f_4c73_9af3_b08c91eacfac)
#define IMGUI_IMM32_ONTHESPOT_SDL_H_HEADER_GUARD_7058bc82_a46f_4c73_9af3_b08c91eacfac 1

#include <SDL.h>

#if defined( __cplusplus )

#if defined( _WIN32 )
template<>
inline BOOL
ImGUIIMMCommunication::subclassify<SDL_Window*>(SDL_Window* window )
{
  SDL_SysWMinfo info{};
  SDL_VERSION(&info.version);
  if (SDL_GetWindowWMInfo(window, &info)) {
    assert(IsWindow(info.info.win.window));
    return this->subclassify( info.info.win.window );
  }
  return FALSE;
}

#endif /* defined( _WIN32 ) */
#endif /* defined( __cplusplus ) */
#endif /* !defined(IMGUI_IMM32_ONTHESPOT_SDL_H_HEADER_GUARD_7058bc82_a46f_4c73_9af3_b08c91eacfac) */
