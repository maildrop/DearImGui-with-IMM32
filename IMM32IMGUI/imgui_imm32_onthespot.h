
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

#include <Windows.h>
#include <commctrl.h>

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
    
    ~IMMCandidateList(){
    }

    IMMCandidateList&
    operator=( const IMMCandidateList& rhv ) = default;
   
    IMMCandidateList&
    operator=( IMMCandidateList&& rhv ) noexcept
    {
      if( this == &rhv ){
        return *this;
      }
      std::swap( list_utf8 , rhv.list_utf8 );
      std::swap( selection , rhv.selection );
      return *this;
    }
    
    static IMMCandidateList cocreate( const CANDIDATELIST* const src , const size_t src_size);
  };
  
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

  ~ImGUIIMMCommunication()
  {
  }

  inline void operator()() {
    if( is_open ){
      ImGuiIO& io = ImGui::GetIO(); 
      ImVec2 window_pos = ImVec2(ImGui::GetCurrentContext()->PlatformImePos.x +1.0f ,  ImGui::GetCurrentContext()->PlatformImePos.y ); // 
      ImVec2 window_pos_pivot = ImVec2(0.0f,0.0f);
      ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
      
      if (ImGui::Begin("IME Composition Window", &(this->is_open),
                       ImGuiWindowFlags_Tooltip|
                       ImGuiWindowFlags_NoNav |
                       ImGuiWindowFlags_NoDecoration | 
                       ImGuiWindowFlags_NoInputs |
                       ImGuiWindowFlags_AlwaysAutoResize |
                       ImGuiWindowFlags_NoSavedSettings ) ){
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.78125f,1.0f,0.1875f, 1.0f) );
        ImGui::Text( static_cast<bool>( comp_conved_utf8 ) ? comp_conved_utf8.get() : u8"" );
        ImGui::PopStyleColor();
        if( static_cast<bool>( comp_target_utf8 ) ){
          ImGui::SameLine(0.0f,0.0f);
          ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.203125f, 0.91796875f, 0.35546875f, 1.0f) );
          ImGui::Text( static_cast<bool>( comp_target_utf8 ) ? comp_target_utf8.get() : u8"" );
          ImGui::PopStyleColor();
        }
        if( static_cast<bool>( comp_unconv_utf8 ) ){
          ImGui::SameLine(0.0f,0.0f);
          ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.78125f,1.0f,0.1875f, 1.0f) );
          ImGui::Text( static_cast<bool>( comp_unconv_utf8 ) ? comp_unconv_utf8.get() : u8"" );
          ImGui::PopStyleColor();
        }
        ImGui::End();
      }
      ImGui::PopStyleVar();
      if( show_ime_candidate_list ){
        if (ImGui::Begin("##IME Candidate Window", nullptr ,
                         ImGuiWindowFlags_Tooltip|
                         ImGuiWindowFlags_NoNav |
                         ImGuiWindowFlags_NoInputs |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoSavedSettings ) ){
          {
            std::vector<const char*> listbox_items ={};
            std::for_each( std::begin( candidate_list.list_utf8 ) , std::end( candidate_list.list_utf8 ),
                           [&](auto &item){
                             listbox_items.push_back( item.c_str() );
                           });
            static int listbox_item_current = 0;
            listbox_item_current = (int)candidate_list.selection;
            
            ImGui::ListBox( u8"##IMECandidateListWindow" , &listbox_item_current ,
                            listbox_items.data() , static_cast<int>( std::size( listbox_items ) ),
                            std::min<int>( 9 , static_cast<int>(std::size( listbox_items ))));
          }
          ImGui::End();
        }
      }
    }
    return;
  }
  
private:
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
    return ::SetWindowSubclass( hWnd , ImGUIIMMCommunication::imm_communication_subClassProc ,
                                reinterpret_cast<UINT_PTR>(ImGUIIMMCommunication::imm_communication_subClassProc),
                                reinterpret_cast<DWORD_PTR>(this) );
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
