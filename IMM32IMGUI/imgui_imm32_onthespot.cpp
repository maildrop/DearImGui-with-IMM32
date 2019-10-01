#include "imgui_imm32_onthespot.h"

#if !defined( VERIFY ) 
# if defined( NDEBUG )
#  define VERIFY( exp ) do{ exp ; }while( 0 )
# else /* defined( NDEBUG ) */
#  define VERIFY( exp ) assert( exp ) 
# endif /* defined( NDEBUG ) */
#endif /* !defined( VERIFY ) */

#include <commctrl.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib" )


ImGUIIMMCommunication::IMMCandidateList
ImGUIIMMCommunication::IMMCandidateList::cocreate( const CANDIDATELIST* const src , const size_t src_size)
{
  assert( nullptr != src );
  assert( sizeof( CANDIDATELIST ) <= src->dwSize );
  assert( src->dwSelection < src->dwCount  );
  
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
    dst.list_utf8.emplace_back( u8"??" );
  }
  dst.selection = src->dwSelection;
  return dst;
}

LRESULT
ImGUIIMMCommunication::imm_communication_subClassProc( HWND hWnd , UINT uMsg , WPARAM wParam, LPARAM lParam ,
                                                       UINT_PTR uIdSubclass , DWORD_PTR dwRefData )
{
  switch( uMsg ){
  case WM_DESTROY:
    {
      if (!RemoveWindowSubclass(hWnd, reinterpret_cast<SUBCLASSPROC>(uIdSubclass), uIdSubclass)) {
        assert(!"RemoveWindowSubclass() failed\n");
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

	  if (!(lParam & ISC_SHOWUICANDIDATEWINDOW)) {
		  OutputDebugStringW(L"ISC_SHOWUICANDIDATEWINDOW not was not set.");
	  }


      lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW |
                  (ISC_SHOWUICANDIDATEWINDOW ) |
                  (ISC_SHOWUICANDIDATEWINDOW << 1) |
                  (ISC_SHOWUICANDIDATEWINDOW << 2) |
                 (ISC_SHOWUICANDIDATEWINDOW << 3) );
				 
	  // lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW);

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
    return 0; // 
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
              assert( 0 < buf_length_in_wchar );
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
                assert( attribute_end == (DWORD)(std::size( attribute_vec ) ) );
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

#if 1
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
                  auto toutf8 = [](const std::wstring& arg )->std::unique_ptr<char[]>{
                    if( arg.empty() ){
                      return std::unique_ptr<char[]>( nullptr );
                    }
                    const int require_byte = WideCharToMultiByte( CP_UTF8 , 0 , arg.c_str() , -1 , nullptr , 0 , NULL , NULL );
                    std::unique_ptr<char[]> utf8buf{ new char[require_byte] };
                    if( require_byte != WideCharToMultiByte( CP_UTF8 , 0 , arg.c_str() , -1, utf8buf.get(), require_byte , NULL , NULL ) ){
                      utf8buf.reset( nullptr );
                    }
                    return utf8buf;
                  };
                  comm.comp_conved_utf8 = toutf8( comp_converted );
                  comm.comp_target_utf8 = toutf8( comp_target ) ;
                  comm.comp_unconv_utf8 = toutf8( comp_unconveted );
                }
              }
            }
            break;
          }
        }
        VERIFY( ImmReleaseContext ( hWnd , hImc ) );
      }
    } // end of WM_IME_COMPOSITION
    return ::DefWindowProc ( hWnd , uMsg , wParam , lParam );

  case WM_IME_NOTIFY:
    {
      switch( wParam ){

      case IMN_OPENCANDIDATE:
		  // 本来ならばIMN_OPENCANDIDATE が送られてきた段階で、show_ime_candidate_list のフラグを立てるのだが
		  // Google IME は、IMN_OPENCANDIDATE を送ってこない（ IMN_CHANGECANDIDATE は送信してくる）
		  // このために、IMN_CHANGECANDIDATE が立ち上がった時に変更する
          comm.show_ime_candidate_list = true; 

		  ; // tear down;
      case IMN_CHANGECANDIDATE:
        {
		  // Google IME 対応用のコード BEGIN 詳細は、IMN_OPENCANDIDATE のコメントを参照
		  if (!comm.show_ime_candidate_list) { 
			  comm.show_ime_candidate_list = true;
		  }
		  // Google IME 対応用のコード END 
          HIMC const hImc = ImmGetContext( hWnd );
          if( hImc ){
            DWORD dwSize = ImmGetCandidateListW( hImc , 0 , NULL , 0 );
            if( dwSize ){
              assert( sizeof(CANDIDATELIST)<=dwSize );
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
        break;
      case IMN_CLOSECANDIDATE:
        {
          comm.show_ime_candidate_list = false;
        }
        break;
      default:
        break;
      }
    }
    return ::DefWindowProc ( hWnd , uMsg , wParam , lParam );

  case WM_IME_REQUEST:
    return ::DefWindowProc ( hWnd , uMsg , wParam , lParam );


  case WM_INPUTLANGCHANGE:
    return ::DefWindowProc ( hWnd , uMsg , wParam , lParam );

  default:
    break;
  }
  return ::DefSubclassProc ( hWnd , uMsg, wParam , lParam );
}

