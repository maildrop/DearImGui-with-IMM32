/**
   Dear ImGui with IME on-the-spot translation routines.
   author: TOGURO Mikito , mit@shalab.net
*/



#pragma once
#if !defined( IMGUI_IMM32_ONTHESPOT_H_UUID_ccfbd514_0a94_4888_a8b8_f065c57c1e70_HEADER_GUARD )
#define IMGUI_IMM32_ONTHESPOT_H_UUID_ccfbd514_0a94_4888_a8b8_f065c57c1e70_HEADER_GUARD 1

//#include <iostream>
#include <memory>
#include <vector>
#include <utility>
#include <string>
#include <type_traits>

#if defined( _WIN32 ) 
#include <tchar.h>
#include <Windows.h>
#include <commctrl.h>
#endif /* defined( _WIN32 ) */

#if !defined( WM_IMGUI_IMM32_COMMAND_BEGIN )
#define WM_IMGUI_IMM32_COMMAND_BEGIN (WM_APP+0x200)
#endif /* !defined( WM_IMGUI_IMM32_COMMAND_BEGIN ) */

struct ImGUIIMMCommunication{

  enum{
    WM_IMGUI_IMM32_COMMAND = WM_IMGUI_IMM32_COMMAND_BEGIN,
    WM_IMGUI_IMM32_END
  };

  enum{
    WM_IMGUI_IMM32_COMMAND_NOP = 0u,
    WM_IMGUI_IMM32_COMMAND_SUBCLASSIFY,
    WM_IMGUI_IMM32_COMMAND_COMPOSITION_COMPLETE,
    WM_IMGUI_IMM32_COMMAND_CLEANUP
  };
  
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
  int request_candidate_list_str_commit; // 1の時に candidate list が更新された後に、次の変換候補へ移る要請をする
  IMMCandidateList candidate_list;
  
  ImGUIIMMCommunication()
    : is_open( false ),
      comp_conved_utf8( nullptr ),
      comp_target_utf8( nullptr ),
      comp_unconv_utf8( nullptr ),
      show_ime_candidate_list( false ),
      request_candidate_list_str_commit( false ),
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
  BOOL subclassify_impl( HWND hWnd );
public:
  template<typename type_t>
  inline BOOL subclassify(type_t hWnd);

  template<>
  inline BOOL subclassify<HWND>( HWND hWnd )
  {
    return subclassify_impl( hWnd );
  }
};

#endif /* IMGUI_IMM32_ONTHESPOT_H_UUID_ccfbd514_0a94_4888_a8b8_f065c57c1e70_HEADER_GUARD */
