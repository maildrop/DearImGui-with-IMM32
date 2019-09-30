#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <locale>
#include <cstdio>
#include <cassert>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_opengl.h>

#include <Windows.h>
#include <commctrl.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib" )


#if !defined( VERIFY ) 
# if defined( NDEBUG )
#  define VERIFY( exp ) do{ exp ; }while( 0 )
# else /* defined( NDEBUG ) */
#  define VERIFY( exp ) assert( exp ) 
# endif /* defined( NDEBUG ) */
#endif /* !defined( VERIFY ) */

/** コモンコントロールの初期化 */
static int common_control_initialize();

static int common_control_initialize()
{
	// 
	// 今 InitCommonControls() を呼び出しているので、 Comctl32.dll は暗黙的にリンクしている
	// しかしながら、 求められているのは InitCommonControlsEx であるので、そちらが存在していればそれを使う 
	HMODULE comctl32 = nullptr;
	if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, L"Comctl32.dll", &comctl32)) {
		return EXIT_FAILURE;
	}

	assert(comctl32 != nullptr);
	if (comctl32) {
		{ // InitCommonControlsEx を使用して初期化を試みる
			typename std::add_pointer< decltype(InitCommonControlsEx) >::type lpfnInitCommonControlsEx =
				reinterpret_cast<typename std::add_pointer< decltype(InitCommonControlsEx) >::type>(GetProcAddress(comctl32, "InitCommonControlsEx"));
			// InitCommonControlsEx が見つかった場合
			if (lpfnInitCommonControlsEx) {
				const INITCOMMONCONTROLSEX initcommoncontrolsex = { sizeof(INITCOMMONCONTROLSEX), ICC_WIN95_CLASSES };
				if (!lpfnInitCommonControlsEx(&initcommoncontrolsex)) {
					assert(!" InitCommonControlsEx(&initcommoncontrolsex) ");
					return EXIT_FAILURE;
				}
				OutputDebugStringW(L"initCommonControlsEx Enable\n");
				return 0;
			}
		}
		{ //InitCommonControls を使用して初期化を試みる
			InitCommonControls();
			OutputDebugStringW(L"initCommonControls Enable\n");
			return 0;
		}
	}
	return 1;
}

struct ImGUIIMMCommunication{
  std::unique_ptr<char[]> comp_conved_utf8;
  std::unique_ptr<char[]> comp_target_utf8;
  std::unique_ptr<char[]> comp_unconv_utf8;
  bool is_open;
  bool show_ime_candidate_list;
  ImGUIIMMCommunication()
    : comp_conved_utf8( nullptr ),
      comp_target_utf8( nullptr ),
      comp_unconv_utf8( nullptr ),
      is_open( false )
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
        OutputDebugStringW( L"IMN_OPENCANDIDATE\n" );
        break;
      case IMN_CHANGECANDIDATE:
        OutputDebugStringW( L"IMN_CHANGECANDIDATE\n" );
        {
          comm.show_ime_candidate_list = true;
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
                  const CANDIDATELIST* cl = reinterpret_cast<CANDIDATELIST*>( candidatelist.data() );
                  (void)(cl);
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
                }
              }
            }
            VERIFY( ImmReleaseContext ( hWnd , hImc ) );
          }
        }
        break;
      case IMN_CLOSECANDIDATE:
        OutputDebugStringW( L"IMN_CLOSECANDIDATE\n" );
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


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
  UNREFERENCED_PARAMETER(lpCmdLine);
  // snprintf 等が wide-multibyte 変換で使うのでローケルの設定は必要
  // ただし windows では mingw の gcc は、ローケルが Cローケルしか受け付けないのでこれは、Visual C++ ぐらいしか使えない
  // (mingw の 仕様のミスなので、当方では修正できません）
  std::locale::global( std::locale("") ); 
    
	common_control_initialize();

	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	// Setup window
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL2_Init();

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'misc/fonts/README.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//

	{
		// フォントの指定
		// フォントの合成を行いたいので MergeMode を有効にする
		const ImFontConfig config = []() {
			ImFontConfig config{};
			config.MergeMode = true;
			config.PixelSnapH = true;
			return config;
		}();

		io.Fonts->AddFontDefault();
		io.Fonts->AddFontFromFileTTF("NotoSansMonoCJKjp-Regular.otf", 16.0f, &config, io.Fonts->GetGlyphRangesJapanese());
	}

	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


    ImGUIIMMCommunication imguiIMMCommunication{};
    VERIFY( imguiIMMCommunication.subclassify( window ) );

	// Main loop
	bool done = false;
	while (!done)
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}


        imguiIMMCommunication(); // IMM popup の描画
        
		// Rendering
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		//glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}

	// Cleanup
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
