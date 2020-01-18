#pragma once

#if !defined(IMGUI_IMM32_ONTHESPOT_SDL_H_HEADER_GUARD_7058bc82_a46f_4c73_9af3_b08c91eacfac)
#define IMGUI_IMM32_ONTHESPOT_SDL_H_HEADER_GUARD_7058bc82_a46f_4c73_9af3_b08c91eacfac 1

#if !defined(IMGUI_IMM32_ONTHESPOT_H_UUID_ccfbd514_0a94_4888_a8b8_f065c57c1e70_HEADER_GUARD)
#include "imgui_imm32_onthespot.h"
#if defined(_MSC_VER)
#pragma message("require include imgui_imm32_onthespot.h before imgui_imm32_onthespot_sdl.h")
#endif /* defined( _MCS_VER ) */
#endif /*!defined(IMGUI_IMM32_ONTHESPOT_H_UUID_ccfbd514_0a94_4888_a8b8_f065c57c1e70_HEADER_GUARD) */

#include <SDL.h>
#include <SDL_syswm.h>

#if defined( __cplusplus )

#if defined( _WIN32 )
template<>
inline BOOL
ImGUIIMMCommunication::subclassify<SDL_Window*>(SDL_Window* window )
{
  SDL_SysWMinfo info{};
  SDL_VERSION(&info.version);
  if (SDL_GetWindowWMInfo(window, &info)) {
    IM_ASSERT(IsWindow(info.info.win.window));
    return this->subclassify( info.info.win.window );
  }
  return FALSE;
}

#endif /* defined( _WIN32 ) */
#endif /* defined( __cplusplus ) */
#endif /* !defined(IMGUI_IMM32_ONTHESPOT_SDL_H_HEADER_GUARD_7058bc82_a46f_4c73_9af3_b08c91eacfac) */
