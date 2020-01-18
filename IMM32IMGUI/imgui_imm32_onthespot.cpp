/**
   Dear ImGui with IME on-the-spot translation routines.
   author: TOGURO Mikito , mit@shalab.net
*/

#include <tchar.h>
#include <windows.h>
#include <commctrl.h>

#include <algorithm> 
#include <cassert>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_imm32_onthespot.h"

#include "imgex.hpp"

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib" )

void 
ImGUIIMMCommunication::operator()()
{
  ImGuiIO& io = ImGui::GetIO(); 

  static ImVec2 window_pos = ImVec2();
  static ImVec2 window_pos_pivot = ImVec2();

  /* 
     #1 Candidate List Window の位置に関する保持
     Candidate List をクリックしたときに、ウィンドウ位置を動かさない。
     クリック後に、TextInputを復帰させる処理
     see #1 
  */
  static ImGuiID candidate_window_root_id = 0;

  static ImGuiWindow* lastTextInputNavWindow = nullptr;
  static ImGuiID lastTextInputActiveId = 0;
  static ImGuiID lastTextInputFocusId = 0;

  // Candidate Window をフォーカスしている時は Window の位置を操作しない 
  if (!( candidate_window_root_id && 
         ((ImGui::GetCurrentContext()->NavWindow ? ImGui::GetCurrentContext()->NavWindow->RootWindow->ID : 0u ) == candidate_window_root_id ) )){

    window_pos = ImVec2(ImGui::GetCurrentContext()->PlatformImePos.x + 1.0f,
                        ImGui::GetCurrentContext()->PlatformImePos.y); // 
    window_pos_pivot = ImVec2(0.0f, 0.0f);
    
    const ImGuiContext* const currentContext = ImGui::GetCurrentContext() ;
    IM_ASSERT( currentContext || !"ImGui::GetCurrentContext() return nullptr.");
    if( currentContext ){
      // mouse press してる間は、ActiveID が切り替わるので、
      if ( !ImGui::IsMouseClicked(0) ) { 
        if ( (currentContext->WantTextInputNextFrame != -1) ? (!!(currentContext->WantTextInputNextFrame)) : false) {
          if( (!!currentContext->NavWindow) &&
              (currentContext->NavWindow->RootWindow->ID != candidate_window_root_id) &&
              (ImGui::GetActiveID() != lastTextInputActiveId) ){
            OutputDebugStringW( L"update lastTextInputActiveId\n");
            lastTextInputNavWindow = ImGui::GetCurrentContext()->NavWindow;
            lastTextInputActiveId = ImGui::GetActiveID();
            lastTextInputFocusId = ImGui::GetFocusID();
          }
        }else{
          if( lastTextInputActiveId != 0 ){
            if( currentContext->WantTextInputNextFrame ){
              OutputDebugStringW( L"update lastTextInputActiveId disabled\n");
            }else{
              OutputDebugStringW( L"update lastTextInputActiveId disabled update\n");
            }
          }
          lastTextInputNavWindow = nullptr;
          lastTextInputActiveId = 0;
          lastTextInputFocusId = 0;
        }
      }
    }
  }

  ImVec2 target_screen_pos = ImVec2(0.0f, 0.0f); // IME Candidate List Window position.

  if( this->is_open ){
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
    if (ImGui::Begin("IME Composition Window", nullptr,
                     ImGuiWindowFlags_Tooltip |
                     ImGuiWindowFlags_NoNav |
                     ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoInputs |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoSavedSettings)) {
      
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.78125f, 1.0f, 0.1875f, 1.0f));
      ImGui::Text(static_cast<bool>(comp_conved_utf8) ? comp_conved_utf8.get() : "");
      ImGui::PopStyleColor();
      
      if (static_cast<bool>(comp_target_utf8)) {
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.203125f, 0.91796875f, 0.35546875f, 1.0f));

        target_screen_pos = ImGui::GetCursorScreenPos();
        target_screen_pos.y += ImGui::GetTextLineHeightWithSpacing();

        ImGui::Text(static_cast<bool>(comp_target_utf8) ? comp_target_utf8.get() : "");
        ImGui::PopStyleColor();
      }
      if (static_cast<bool>(comp_unconv_utf8)) {
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.78125f, 1.0f, 0.1875f, 1.0f));
        ImGui::Text(static_cast<bool>(comp_unconv_utf8) ? comp_unconv_utf8.get() : "");
        ImGui::PopStyleColor();
      }
      /*
        ImGui::SameLine();
        ImGui::Text("%u %u", 
        candidate_window_root_id ,
        ImGui::GetCurrentContext()->NavWindow ? ImGui::GetCurrentContext()->NavWindow->RootWindow->ID : 0u);
      */
      ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    }
    ImGui::End();
    ImGui::PopStyleVar();
    
    /* Draw Candidate List */
    if( show_ime_candidate_list && !candidate_list.list_utf8.empty()){
      
      std::vector<const char*> listbox_items ={};
      
      /* ページに分割します */
      // TODO:candidate_window_num の値に注意 0 除算の可能性がある。
      IM_ASSERT( candidate_window_num );
      int candidate_page = ((int)candidate_list.selection) / candidate_window_num;
      int candidate_selection = ((int)candidate_list.selection) % candidate_window_num;
      
      auto begin_ite = std::begin(candidate_list.list_utf8);
      std::advance(begin_ite, candidate_page * candidate_window_num);
      auto end_ite = begin_ite;
      {
        auto the_end = std::end(candidate_list.list_utf8);
        for (int i = 0; end_ite != the_end && i < candidate_window_num; ++i) {
          std::advance(end_ite, 1);
        }
      }
      
      std::for_each( begin_ite , end_ite , 
                     [&](auto &item){
                       listbox_items.push_back( item.c_str() );
                     });

      /* もし candidate window が画面の外に出ることがあるのならば、上に表示する */
      const float candidate_window_height =
        (( ImGui::GetStyle().FramePadding.y * 2) +
         (( ImGui::GetTextLineHeightWithSpacing() ) * ((int)std::size( listbox_items ) +2 )));

      if( io.DisplaySize.y < (target_screen_pos.y + candidate_window_height) ){
        target_screen_pos.y -=
          ImGui::GetTextLineHeightWithSpacing() + candidate_window_height;
      }
      
      ImGui::SetNextWindowPos(target_screen_pos, ImGuiCond_Always, window_pos_pivot);

      if( ImGui::Begin( "##Overlay-IME-Candidate-List-Window" ,
                        &show_ime_candidate_list ,
                        ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoDecoration |
                        ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_NoInputs | 
                        ImGuiWindowFlags_NoSavedSettings |
                        ImGuiWindowFlags_NoFocusOnAppearing |
                        ImGuiWindowFlags_NoNav ) ){
        if( ImGui::ListBoxHeader( "##IMECandidateListWindow" ,
                                  static_cast<int>(std::size( listbox_items )),
                                  static_cast<int>(std::size( listbox_items )))){
          
          int i = 0; // for の最後で、i をインクリメントしているので、注意 
          for( const char* &listbox_item : listbox_items ){
            if (ImGui::Selectable (listbox_item, (i == candidate_selection))) {

              /* candidate list selection */

              if (ImGui::IsWindowFocused (ImGuiFocusedFlags_RootAndChildWindows) &&
                  !ImGui::IsAnyItemActive () &&
                  !ImGui::IsMouseClicked (0)) {
                if (lastTextInputActiveId && lastTextInputFocusId) {
                  ImGui::SetActiveID (lastTextInputActiveId, lastTextInputNavWindow);
                  ImGui::SetFocusID (lastTextInputFocusId, lastTextInputNavWindow);
                }
              }

              /*
                ImmNotifyIME (hImc, NI_SELECTCANDIDATESTR, 0, candidate_page* candidate_window_num + i)); をしたいのだが、
                Vista 以降 ImmNotifyIME は NI_SELECTCANDIDATESTR はサポートされない。
                @see IMM32互換性情報.doc from Microsoft
                
                そこで、DXUTguiIME.cpp (かつて使われていた DXUT の gui IME 処理部 現在は、deprecated 扱いで、
                https://github.com/microsoft/DXUT で確認出来る
                当該のコードは、https://github.com/microsoft/DXUT/blob/master/Optional/DXUTguiIME.cpp )
                を確認したところ
                
                L.380から で Candidate List をクリックしたときのコードがある
                
                どうしているかというと SendKey で、矢印カーソルキーを送ることで、Candidate Listからの選択を行っている。
                （なんということ？！）
                
                これを根拠に SendKey を利用したコードを作成する。
              */
              {
                /*
                  これで、選択された変換候補が末尾の場合は確定、
                  そうでない場合は、次の変換文節を選択させたいのであるが、
                  keybd_event で状態を送っているので、PostMessage でその処理を遅らせる
                  この request_candidate_list_str_commit は、 WM_IME_COMPOSITION の最後でチェックされ
                  WM_APP + 200 を PostMessage して、そこで実際の確定動作が行われる。
                */
                if (candidate_selection == i) {
                  /* 確定動作 */
                  OutputDebugStringW (L"complete\n");
                  this->request_candidate_list_str_commit = 1;
                }else{
                  const BYTE nVirtualKey = (candidate_selection < i) ? VK_DOWN : VK_UP;
                  const size_t nNumToHit = abs (candidate_selection - i);
                  for (size_t hit = 0; hit < nNumToHit; ++hit) {
                    keybd_event (nVirtualKey, 0, 0, 0);
                    keybd_event (nVirtualKey, 0, KEYEVENTF_KEYUP, 0);
                  }
                  this->request_candidate_list_str_commit = (int)nNumToHit;
                }
              }
              
            }
            ++i;
          }
          ImGui::ListBoxFooter ();
        }
        ImGui::Text("%d/%d",
                    candidate_list.selection + 1, static_cast<int>(std::size(candidate_list.list_utf8)));
#if defined( _DEBUG )
        ImGui::SameLine();
        ImGui::TextColored( ImVec4(1.0f, 0.8f, 0.0f, 1.0f) , "%s",
# if defined( UNICODE )
                            u8"DEBUG (UNICODE)"
# else 
                            u8"DEBUG (MBCS)"
# endif /* defined( UNICODE ) */
                            );
#endif /* defined( DEBUG ) */
        // #1 ここで作るウィンドウがフォーカスを持ったときには、ウィンドウの位置を変更してはいけない。
        candidate_window_root_id = ImGui::GetCurrentWindowRead()->RootWindow->ID;
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
      }
      ImGui::End();
    }
  }

  /*
    io.WantTextInput は単純に信用は出来ない。
    テキストインプットウィジットから、他のウィンドウ上でマウスボタンを押し下げると、
    WantTextInput が 有効なまま、他のウィンドウとウイジットがアクティブになる
    （特に、 IME candidate window のウイジットが絡むと、
    押す WantTextInput が有効なまま、フォーカスが、IME candidate Window に移動する。
    次のフレームで、
    IME candidate window は、テキストインプット有効ではないので、WantTextInput がオフになる
    するとここで、IME が無効化されるという現象が起きていた。）

    これは、直感的な挙動に反するので修正を行うが、 無効にする際には、
    何かのウイジットがウィンドウフォーカスを得ていない場合
    有効にする際には WantTextInput が真の時 
    と非対称な形にすることで 違和感を軽減する。
  */
  if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) {
    if (io.ImeWindowHandle) {
      IM_ASSERT( IsWindow( static_cast<HWND>( io.ImeWindowHandle )));
      (void)( ImmAssociateContext (static_cast<HWND>(io.ImeWindowHandle) , HIMC(0)  ));
    }
  }
  if (io.WantTextInput) {
    if (io.ImeWindowHandle) {
      IM_ASSERT( IsWindow( static_cast<HWND>( io.ImeWindowHandle )));
      /*
        The second argument of ImmAssociateContextEx is
        Changed from IN to _In_ between 10.0.10586.0-Windows 10 and 10.0.14393.0-Windows.
        
        https://abi-laboratory.pro/compatibility/Windows_10_1511_10586.494_to_Windows_10_1607_14393.0/x86_64/headers_diff/imm32.dll/diff.html
        
        This is strange, but HIMC is probably not zero because this change was intentional. 
        However, the document states that HIMC is ignored if the flag IACE_DEFAULT is used.
        
        https://docs.microsoft.com/en-us/windows/win32/api/immdev/nf-immdev-immassociatecontextex  

        Passing HIMC appropriately when using IACE_DEFAULT causes an error and returns to 0
      */
      VERIFY(ImmAssociateContextEx (static_cast<HWND>(io.ImeWindowHandle), HIMC(0) , IACE_DEFAULT));
    }
  }

  return;
}

ImGUIIMMCommunication::IMMCandidateList
ImGUIIMMCommunication::IMMCandidateList::cocreate( const CANDIDATELIST* const src , const size_t src_size)
{
  IM_ASSERT( nullptr != src );
  IM_ASSERT( sizeof( CANDIDATELIST ) <= src->dwSize );
  IM_ASSERT( src->dwSelection < src->dwCount  );
  
  IMMCandidateList dst{};
  if(! (sizeof( CANDIDATELIST) < src->dwSize )){
    return dst;
  }
  if(! (src->dwSelection < src->dwCount ) ){
    return dst;
  }
  const char* const baseaddr = reinterpret_cast< const char* >( src );
  
  for( size_t i = 0; i < src->dwCount ; ++i ){
    const wchar_t * const item = reinterpret_cast<const wchar_t*>( baseaddr + src->dwOffset[i] );
    const int require_byte = WideCharToMultiByte( CP_UTF8 , 0 , item , -1 , nullptr , 0 , NULL , NULL );
    if( 0 < require_byte ){
      std::unique_ptr<char[]> utf8buf{ new char[require_byte] };
      if( require_byte == WideCharToMultiByte( CP_UTF8 , 0 , item , -1 , utf8buf.get() , require_byte , NULL, NULL ) ){
        dst.list_utf8.emplace_back( utf8buf.get() );
        continue;
      }
    }
    dst.list_utf8.emplace_back( "??" );
  }
  dst.selection = src->dwSelection;
  return dst;
}

bool
ImGUIIMMCommunication::update_candidate_window(HWND hWnd)
{
  IM_ASSERT( IsWindow( hWnd ) );
  bool result = false;
  HIMC const hImc = ImmGetContext( hWnd );
  if( hImc ){
    DWORD dwSize = ImmGetCandidateListW( hImc , 0 , NULL , 0 );
      
    if( dwSize ){
      IM_ASSERT( sizeof(CANDIDATELIST)<=dwSize );
      if( sizeof(CANDIDATELIST)<=dwSize ){ // dwSize は最低でも struct CANDIDATELIST より大きくなければならない
        
        std::vector<char> candidatelist( (size_t)dwSize );
        if( (DWORD)(std::size( candidatelist ) * sizeof( typename decltype( candidatelist )::value_type ) )
            == ImmGetCandidateListW( hImc , 0 , 
                                     reinterpret_cast<CANDIDATELIST*>(candidatelist.data()),
                                     (DWORD)(std::size( candidatelist ) * sizeof( typename decltype( candidatelist )::value_type ) )) ){
          const CANDIDATELIST* const cl = reinterpret_cast<CANDIDATELIST*>( candidatelist.data() );
          candidate_list = std::move(IMMCandidateList::cocreate( cl , dwSize ));
          result = true;
#if 0  /* for IMM candidate window debug BEGIN*/
          {
            wchar_t dbgbuf[1024];
            _snwprintf_s( dbgbuf , sizeof( dbgbuf )/sizeof( dbgbuf[0] ),
                          L" dwSize = %d , dwCount = %d , dwSelection = %d\n",
                          cl->dwSize ,
                          cl->dwCount,
                          cl->dwSelection);
            OutputDebugStringW( dbgbuf );
            for( DWORD i = 0; i < cl->dwCount ; ++i ){
              _snwprintf_s( dbgbuf , sizeof( dbgbuf )/sizeof( dbgbuf[0] ),
                            L"%d%s: %s\n" ,
                            i ,(cl->dwSelection == i ? L"*" : L" "),
                            (wchar_t*)( candidatelist.data() + cl->dwOffset[i] ) );
              OutputDebugStringW( dbgbuf );
            }
          }
#endif /* for IMM candidate window debug END */
          
        }
      }
    }
    VERIFY( ImmReleaseContext ( hWnd , hImc ) );
  }
  return result;
}

LRESULT
ImGUIIMMCommunication::imm_communication_subClassProc( HWND hWnd , UINT uMsg , WPARAM wParam, LPARAM lParam ,
                                                       UINT_PTR uIdSubclass , DWORD_PTR dwRefData )
{
  switch( uMsg ){
  case WM_DESTROY:
    {
      VERIFY( ImmAssociateContextEx (hWnd, HIMC(0) , IACE_DEFAULT) );
      if (!RemoveWindowSubclass(hWnd, reinterpret_cast<SUBCLASSPROC>(uIdSubclass), uIdSubclass)) {
        IM_ASSERT(!"RemoveWindowSubclass() failed\n");
      }
    }
    break;
  default:
    if( dwRefData ){
      return imm_communication_subClassProc_implement ( hWnd, uMsg , wParam , lParam,
                                                        uIdSubclass ,*reinterpret_cast<ImGUIIMMCommunication*>( dwRefData ) );
    }
  }
  return ::DefSubclassProc ( hWnd , uMsg , wParam , lParam );
}

LRESULT
ImGUIIMMCommunication::imm_communication_subClassProc_implement( HWND hWnd , UINT uMsg , WPARAM wParam, LPARAM lParam ,
                                                                 UINT_PTR uIdSubclass , ImGUIIMMCommunication& comm )
{
  switch( uMsg ){
  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
    if( comm.is_open ){
      return 0;
    }
    break;
    
  case WM_IME_SETCONTEXT:
    { /* 各ビットを落とす */
      lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW |
                  (ISC_SHOWUICANDIDATEWINDOW ) |
                  (ISC_SHOWUICANDIDATEWINDOW << 1) |
                  (ISC_SHOWUICANDIDATEWINDOW << 2) |
                  (ISC_SHOWUICANDIDATEWINDOW << 3) );
    }
    return ::DefWindowProc( hWnd , uMsg , wParam , lParam );
  case WM_IME_STARTCOMPOSITION:
    {
      /* このメッセージは 「IME」に 変換ウィンドウの表示を要求することを アプリケーションに通知します
         アプリケーションが 変換文字列の表示を処理する際には、このメッセージを処理しなければいけません。
         DefWindowProc 関数にこのメッセージを処理させると、デフォルトの IME Window にメッセージが伝わります。
         （つまり 自前で変換ウィンドウを描画する時には、DefWindowPrc に渡さない）
      */
      comm.is_open = true;
    }
    return 1;
  case WM_IME_ENDCOMPOSITION:
    {
      comm.is_open = false;
    }
    return DefSubclassProc ( hWnd , uMsg , wParam , lParam );
  case WM_IME_COMPOSITION:
    {
      HIMC const hImc = ImmGetContext( hWnd );
      if( hImc ){
        if( lParam & GCS_RESULTSTR ){
          comm.comp_conved_utf8 = nullptr;
          comm.comp_target_utf8 = nullptr;
          comm.comp_unconv_utf8 = nullptr;
          comm.show_ime_candidate_list = false;
        }
        if( lParam & GCS_COMPSTR ){
        
          /* 一段階目で IME から ワイド文字で文字列をもらってくる */
          /* これはバイト単位でやりとりするので、注意 */
          const DWORD compstr_length_in_byte = ImmGetCompositionStringW( hImc , GCS_COMPSTR , nullptr , 0 ) ;
          switch( compstr_length_in_byte ){
          case IMM_ERROR_NODATA:
          case IMM_ERROR_GENERAL:
            break;
          default:
            {
              /* バイト単位でもらってきたので、wchar_t 単位に直して、 null文字の余裕を加えてバッファを用意する */
              size_t const buf_length_in_wchar = ( size_t(compstr_length_in_byte) / sizeof( wchar_t ) ) + 1 ;
              IM_ASSERT( 0 < buf_length_in_wchar );
              std::unique_ptr<wchar_t[]> buf{ new wchar_t[buf_length_in_wchar] };
              if( buf ){
                //std::fill( &buf[0] , &buf[buf_length_in_wchar-1] , L'\0' );
                const LONG buf_length_in_byte = LONG( buf_length_in_wchar * sizeof( wchar_t ) );
                const DWORD l = ImmGetCompositionStringW( hImc , GCS_COMPSTR ,
                                                          (LPVOID)(buf.get()) , buf_length_in_byte );

                const DWORD attribute_size = ImmGetCompositionStringW( hImc , GCS_COMPATTR , NULL, 0 );
                std::vector<char> attribute_vec( attribute_size , 0 );
                const DWORD attribute_end =
                  ImmGetCompositionStringW( hImc , GCS_COMPATTR , attribute_vec.data() , (DWORD)std::size( attribute_vec ));
                IM_ASSERT( attribute_end == (DWORD)(std::size( attribute_vec ) ) );
                {
                  std::wstring comp_converted;
                  std::wstring comp_target;
                  std::wstring comp_unconveted;
                  size_t begin = 0;
                  size_t end = 0;
                  // 変換済みを取り出す
                  for( end = begin ; end < attribute_end; ++end ){
                    if( (ATTR_TARGET_CONVERTED == attribute_vec[end] ||
                         ATTR_TARGET_NOTCONVERTED == attribute_vec[end] ) ){
                      break;
                    }else{
                      comp_converted.push_back( buf[ end ] );
                    }
                  }
                  // 変換済みの領域[begin,end) 

                  // 変換中の文字列を取り出す
                  for( begin = end ; end < attribute_end; ++end ){
                    if( !(ATTR_TARGET_CONVERTED == attribute_vec[end] ||
                          ATTR_TARGET_NOTCONVERTED == attribute_vec[end] ) ){
                      break;
                    }else{
                      comp_target.push_back( buf[end] );
                    }
                  }
                  // 変換中の領域 [begin,end)

                  // 未変換の文字列を取り出す
                  for( ; end < attribute_end ; ++end ){
                    comp_unconveted.push_back( buf[end] );
                  }

#if 0
                  {
                    wchar_t dbgbuf[1024];
                    _snwprintf_s( dbgbuf , sizeof( dbgbuf ) / sizeof( dbgbuf[0] ) ,
                                  L"attribute_size = %d \"%s[%s]%s\"\n",
                                  attribute_size ,
                                  comp_converted.c_str() ,
                                  comp_target.c_str() ,
                                  comp_unconveted.c_str() );
                    OutputDebugStringW( dbgbuf );
                  }
#endif
                  // それぞれ UTF-8 に変換するためにラムダ関数用意する
                  /*
                    std::wstring を std::unique_ptr<char[]> に UTF-8 のnull 終端文字列にして変換する
                    引数が空文字列の場合は nullptr が入る
                  */
                  auto to_utf8_pointer = [](const std::wstring& arg )->std::unique_ptr<char[]>{
                    if( arg.empty() ){
                      return std::unique_ptr<char[]>( nullptr );
                    }
                    const int require_byte = WideCharToMultiByte( CP_UTF8 , 0 , arg.c_str() , -1 , nullptr , 0 , NULL , NULL );
                    if ( 0 == require_byte) {
                      const DWORD lastError = GetLastError ();
                      (void)(lastError);
                      IM_ASSERT (ERROR_INSUFFICIENT_BUFFER != lastError);
                      IM_ASSERT (ERROR_INVALID_FLAGS != lastError);
                      IM_ASSERT (ERROR_INVALID_PARAMETER != lastError);
                      IM_ASSERT (ERROR_NO_UNICODE_TRANSLATION != lastError);
                    }
                    IM_ASSERT (0 != require_byte);
                    if ( !(0 < require_byte) ) {
                      return std::unique_ptr<char[]> (nullptr);
                    }

                    std::unique_ptr<char[]> utf8buf{ new char[require_byte] };

                    const int conversion_result =
                    WideCharToMultiByte (CP_UTF8, 0, arg.c_str (), -1, utf8buf.get (), require_byte, NULL, NULL);
                    if (conversion_result == 0) {
                      const DWORD lastError = GetLastError ();
                      (void)(lastError);
                      IM_ASSERT (ERROR_INSUFFICIENT_BUFFER != lastError);
                      IM_ASSERT (ERROR_INVALID_FLAGS != lastError);
                      IM_ASSERT (ERROR_INVALID_PARAMETER != lastError);
                      IM_ASSERT (ERROR_NO_UNICODE_TRANSLATION != lastError);
                    }

                    IM_ASSERT (require_byte == conversion_result);
                    if (require_byte != conversion_result) {
                      utf8buf.reset( nullptr );
                    }
                    return utf8buf;
                  };

                  comm.comp_conved_utf8 = to_utf8_pointer( comp_converted );
                  comm.comp_target_utf8 = to_utf8_pointer( comp_target ) ;
                  comm.comp_unconv_utf8 = to_utf8_pointer( comp_unconveted );

                  /* Google IME は GCS_COMPSTR の更新が行われたときには、当然 IMN_CHANGECANDIDATE
                     が行われたものとして、送ってこない。
                     なので、ここで　Candidate List の更新を行うことを要求する */

                  // comp_target が空の場合は消去する
                  if( !static_cast<bool>( comm.comp_target_utf8 ) ){
                    comm.candidate_list.clear(); 
                  }else{
                    // candidate_list の取得に失敗したときは消去する
                    if( !comm.update_candidate_window( hWnd ) ){
                      comm.candidate_list.clear(); 
                    }
                  }
                }
              }
            }
            break;
          }
        }
        VERIFY( ImmReleaseContext ( hWnd , hImc ) );
      }


    } // end of WM_IME_COMPOSITION

#if defined( UNICODE )
    // UNICODE 構成の時には、DefWindowProc で直接IMEに吸収させて大丈夫
    return ::DefWindowProc ( hWnd , uMsg , wParam , lParam );
    // マルチバイト 構成の時には、Window サブクラスの プロシージャが処理するので、 DefSubclassProc する必要がある。
#else
    return ::DefSubclassProc ( hWnd , uMsg , wParam , lParam );
#endif 

  case WM_IME_NOTIFY:
    {
      switch( wParam ){

      case IMN_OPENCANDIDATE:
        // 本来ならばIMN_OPENCANDIDATE が送られてきた段階で、show_ime_candidate_list のフラグを立てるのだが
        // Google IME は、IMN_OPENCANDIDATE を送ってこない（ IMN_CHANGECANDIDATE は送信してくる）
        // このために、IMN_CHANGECANDIDATE が立ち上がった時に変更する
        comm.show_ime_candidate_list = true; 
#if 0
        if (IMN_OPENCANDIDATE == wParam) {
          OutputDebugStringW(L"IMN_OPENCANDIDATE\n");
        }
#endif
        ; // tear down;
      case IMN_CHANGECANDIDATE:
        {
#if 0
          if (IMN_CHANGECANDIDATE == wParam) {
            OutputDebugStringW(L"IMN_CHANGECANDIDATE\n");
          }
#endif
          
          // Google IME 対応用のコード BEGIN 詳細は、IMN_OPENCANDIDATE のコメントを参照
          if (!comm.show_ime_candidate_list) {
            comm.show_ime_candidate_list = true;
          }
          // Google IME 対応用のコード END 

          
          HIMC const hImc = ImmGetContext( hWnd );
          if( hImc ){
            DWORD dwSize = ImmGetCandidateListW( hImc , 0 , NULL , 0 );
            if( dwSize ){
              IM_ASSERT( sizeof(CANDIDATELIST)<=dwSize );
              if( sizeof(CANDIDATELIST)<=dwSize ){ // dwSize は最低でも struct CANDIDATELIST より大きくなければならない
                
                (void)(lParam);
                std::vector<char> candidatelist( (size_t)dwSize );
                if( (DWORD)(std::size( candidatelist ) * sizeof( typename decltype( candidatelist )::value_type ) )
                    == ImmGetCandidateListW( hImc , 0 , 
                                             reinterpret_cast<CANDIDATELIST*>(candidatelist.data()),
                                             (DWORD)(std::size( candidatelist ) * sizeof( typename decltype( candidatelist )::value_type ) )) ){
                  const CANDIDATELIST* const cl = reinterpret_cast<CANDIDATELIST*>( candidatelist.data() );
                  comm.candidate_list = std::move(IMMCandidateList::cocreate( cl , dwSize ));

#if 0  /* for IMM candidate window debug BEGIN*/
                  {
                    wchar_t dbgbuf[1024];
                    _snwprintf_s( dbgbuf , sizeof( dbgbuf )/sizeof( dbgbuf[0] ),
                                  L"lParam = %lld, dwSize = %d , dwCount = %d , dwSelection = %d\n",
                                  lParam,
                                  cl->dwSize ,
                                  cl->dwCount,
                                  cl->dwSelection);
                    OutputDebugStringW( dbgbuf );
                    for( DWORD i = 0; i < cl->dwCount ; ++i ){
                      _snwprintf_s( dbgbuf , sizeof( dbgbuf )/sizeof( dbgbuf[0] ),
                                    L"%d%s: %s\n" ,
                                    i ,(cl->dwSelection == i ? L"*" : L" "),
                                    (wchar_t*)( candidatelist.data() + cl->dwOffset[i] ) );
                      OutputDebugStringW( dbgbuf );
                    }
                  }
#endif /* for IMM candidate window debug END */

                }
              }
            }
            VERIFY( ImmReleaseContext ( hWnd , hImc ) );
          }
        }

        IM_ASSERT (0 <= comm.request_candidate_list_str_commit );
        if (comm.request_candidate_list_str_commit) {
          if( comm.request_candidate_list_str_commit == 1){
            VERIFY(PostMessage (hWnd, WM_IMGUI_IMM32_COMMAND, WM_IMGUI_IMM32_COMMAND_COMPOSITION_COMPLETE , 0));
          }
          --(comm.request_candidate_list_str_commit);
        }

        break;
      case IMN_CLOSECANDIDATE:
        {
          //OutputDebugStringW(L"IMN_CLOSECANDIDATE\n");
          comm.show_ime_candidate_list = false;
        }
        break;
      default:
        break;
      }
    }
    return ::DefWindowProc ( hWnd , uMsg , wParam , lParam );

  case WM_IME_REQUEST:
    return ::DefWindowProc (hWnd, uMsg, wParam, lParam);

  case WM_INPUTLANGCHANGE:
    return ::DefWindowProc (hWnd, uMsg, wParam, lParam);

  case WM_IMGUI_IMM32_COMMAND:
    {
      switch( wParam ){
      case WM_IMGUI_IMM32_COMMAND_NOP: // NOP
        return 1;
      case WM_IMGUI_IMM32_COMMAND_SUBCLASSIFY:
        {
          ImGuiIO& io = ImGui::GetIO(); 
          if( io.ImeWindowHandle ){
            IM_ASSERT( IsWindow( static_cast<HWND>( io.ImeWindowHandle )));
            VERIFY(ImmAssociateContextEx (static_cast<HWND>(io.ImeWindowHandle) , nullptr , IACE_IGNORENOCONTEXT ));
          }
        }
        return 1;
      case WM_IMGUI_IMM32_COMMAND_COMPOSITION_COMPLETE:
        if (!static_cast<bool>(comm.comp_unconv_utf8) ||
            '\0' == *(comm.comp_unconv_utf8.get ())) {
          /* 
             There is probably no unconverted string after the conversion target.  
             However, since there is now a cursor operation in the
             keyboard buffer, the confirmation operation needs to be performed
             after the cursor operation is completed.  For this purpose, an enter
             key, which is a decision operation, is added to the end of the
             keyboard buffer.
          */
#if 0
          /*
            Here, the expected behavior is to perform the conversion completion
            operation. 
            However, there is a bug that the TextInput loses the focus
            and the conversion result is lost when the conversion
            complete operation is performed.
            
            To work around this bug, disable it.
            This is because the movement of the focus of the widget and
            the accompanying timing are complicated relationships.
          */
          HIMC const hImc = ImmGetContext (hWnd);
          if (hImc) {
            VERIFY (ImmNotifyIME (hImc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0));
            VERIFY (ImmReleaseContext (hWnd, hImc));
          }
#else
          keybd_event (VK_RETURN, 0, 0, 0);
          keybd_event (VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
#endif
        } else {
          /* Do this to close the candidate window without ending composition. */
          /*
            keybd_event (VK_RIGHT, 0, 0, 0);
            keybd_event (VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
            
            keybd_event (VK_LEFT, 0, 0, 0);
            keybd_event (VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
          */
          /* 
             Since there is an unconverted string after the conversion
             target, press the right key of the keyboard to convert the
             next clause to IME.
          */
          keybd_event (VK_RIGHT, 0, 0, 0);
          keybd_event (VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
        }
        return 1;
      default:
        break;
      }
    }
    return 1;
  default:
    break;
  }
  return ::DefSubclassProc ( hWnd , uMsg, wParam , lParam );
}

BOOL
ImGUIIMMCommunication::subclassify_impl(HWND hWnd)
{
  IM_ASSERT(IsWindow(hWnd));
  if (!IsWindow(hWnd)) {
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
  if (::SetWindowSubclass(hWnd, ImGUIIMMCommunication::imm_communication_subClassProc,
                          reinterpret_cast<UINT_PTR>(ImGUIIMMCommunication::imm_communication_subClassProc),
                          reinterpret_cast<DWORD_PTR>(this))) {
    /* 
       I want to close IME once by calling imgex::imm_associate_context_disable()
       
       However, at this point, default IMM Context may not have been initialized by User32.dll yet.
       Therefore, a strategy is to post the message and set the IMMContext after the message pump is turned on.
    */
    return PostMessage( hWnd, WM_IMGUI_IMM32_COMMAND , WM_IMGUI_IMM32_COMMAND_SUBCLASSIFY , 0u ) ;
  }
  return FALSE;
}
