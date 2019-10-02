/**
   Dear ImGui with IME on-the-spot translation routines.
   author: TOGURO Mikito , mit@shalab.net
 */
#pragma once
#if !defined( IMGUI_IMM32_ONTHESPOT_H_UUID_ccfbd514_0a94_4888_a8b8_f065c57c1e70_HEADER_GUARD )
#define IMGUI_IMM32_ONTHESPOT_H_UUID_ccfbd514_0a94_4888_a8b8_f065c57c1e70_HEADER_GUARD 1

#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <type_traits>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"

#include <SDL.h>
#include <SDL_syswm.h>

#if defined( _WIN32 ) 
#include <Windows.h>
#include <commctrl.h>
#endif /* defined( _WIN32 ) */

struct ImGUIIMMCommunication{

  struct IMMCandidateList{
    std::vector<std::string> list_utf8;
    std::vector<std::string>::size_type selection;

    IMMCandidateList()
      : list_utf8{}, selection(0){
    }
    IMMCandidateList( const IMMCandidateList& rhv ) = default;
    IMMCandidateList( IMMCandidateList&& rhv ) noexcept
      : list_utf8() , selection( 0 ){
      *this = std::move( rhv );
    }
    
    ~IMMCandidateList() = default;

    inline IMMCandidateList&
    operator=( const IMMCandidateList& rhv ) = default;
   
    inline IMMCandidateList&
    operator=( IMMCandidateList&& rhv ) noexcept
    {
      if( this == &rhv ){
        return *this;
      }
      std::swap( list_utf8 , rhv.list_utf8 );
      std::swap( selection , rhv.selection );
      return *this;
    }
    inline void clear(){
      list_utf8.clear();
      selection = 0;
    }
    static IMMCandidateList cocreate( const CANDIDATELIST* const src , const size_t src_size);
  };
  
  static constexpr int candidate_window_num = 9;

  bool is_open;
  std::unique_ptr<char[]> comp_conved_utf8;
  std::unique_ptr<char[]> comp_target_utf8;
  std::unique_ptr<char[]> comp_unconv_utf8;
  bool show_ime_candidate_list;
  IMMCandidateList candidate_list;
  
  ImGUIIMMCommunication()
    : is_open( false ),
      comp_conved_utf8( nullptr ),
      comp_target_utf8( nullptr ),
      comp_unconv_utf8( nullptr ),
      show_ime_candidate_list( false ),
      candidate_list()
  {
  }

  ~ImGUIIMMCommunication() = default;
   void operator()();

private:
  bool update_candidate_window(HWND hWnd);

  static LRESULT
  WINAPI imm_communication_subClassProc( HWND hWnd , UINT uMsg , WPARAM wParam, LPARAM lParam ,
                                  UINT_PTR uIdSubclass , DWORD_PTR dwRefData );
  static LRESULT
  imm_communication_subClassProc_implement( HWND hWnd , UINT uMsg , WPARAM wParam, LPARAM lParam ,
                                            UINT_PTR uIdSubclass , ImGUIIMMCommunication& comm);
public:
  inline BOOL
  subclassify( HWND hWnd )
  {
    assert( IsWindow( hWnd ) );
    if(! IsWindow( hWnd ) ){
      return FALSE;
    }

    /* IME 制御用 imgui_imm32_onthespot では、
       TextWidget がフォーカスを失ったときに io.WantTextInput が true -> off になるので
       この時にIMEのステータスを見て、開いていれば閉じる 
       @see ImGUIIMMCommunication::operator()() の先頭

       Dear ImGui の ImGui::IO::ImeWindowHandle は元々 CompositionWindowの位置を指定するために
       使っていたのでその目的に合致する 

       しかしながらこの方法は、ターゲットになるOSのウィンドウが複数になると、破綻するので筋が良くない
    */
    ImGui::GetIO().ImeWindowHandle = static_cast<void*>(hWnd);
    if( ::SetWindowSubclass( hWnd , ImGUIIMMCommunication::imm_communication_subClassProc ,
                             reinterpret_cast<UINT_PTR>(ImGUIIMMCommunication::imm_communication_subClassProc),
                             reinterpret_cast<DWORD_PTR>(this) ) ){
      HIMC hImc = ImmAssociateContext( hWnd , nullptr );
      if( ! ::SetProp( hWnd , TEXT( "DearImGuiIMEContext") , (HANDLE) hImc ) ){
        assert( !"SetProp failed" );
        ImmAssociateContext( hWnd , hImc );
      }
      return TRUE;
    }
    return FALSE;
  }
  
  inline BOOL
  subclassify( SDL_Window* window )
  {
    SDL_SysWMinfo info{};
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(window, &info)) {
      assert(IsWindow(info.info.win.window));
      return this->subclassify( info.info.win.window );
    }
    return FALSE;
  }
  
};

#endif /* IMGUI_IMM32_ONTHESPOT_H_UUID_ccfbd514_0a94_4888_a8b8_f065c57c1e70_HEADER_GUARD */
