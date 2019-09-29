#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include <iostream>
#include <cstdio>
#include <cassert>
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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(lpCmdLine);

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


	// IME のメッセージを取得してごにょごにょするために頑張る
	/*
	   SDL は Widget が無いので変換ウィンドウの位置を知ることが出来ないし、ImGUI の IME 実装は、中途半端になっている
	   このためもしやるのであれば、SetWindowSubclass を使う方法で WM_IME_* をSDL から分離する方法が良い気がする

	   SDL_SetWindowsMessageHook は、メッセージポンプ上で処理をするので、IMEからのSendMessage で送出されるメッセージを
	   取得出来ない。自スレッドからSendMessageで送出されるメッセージは、直接ウィンドウプロシージャに送られるし、
	   メッセージボックスやシステムウィンドウをつかんだ時などは、システム提供のメッセージポンプが動作して、ウィンドウプロシージャより先しか
	   動作が保証できない。
	   このために、メッセージポンプ上の処理は信頼性が無い
	   ぶっちゃけ TranslateAccelerator を呼び出すか呼び出さないかぐらいにしか役には立たないし、
	   それをする時には、

	   if( !TranslateAccelerator( hwnd , haccel , &msg ) ){
		 TranslateMessage( &msg );
		 DispatchMessage( &msg );
	   }

	   のようにTranslateAccelerator() が true を返した時には、TranslateMessage DispatchMessage を呼び出すべきでは無い
	   そして、 SDL_SetWindowsMessageHook はそのメッセージを後ろに送るのを阻止するという方法が無い。

	   SDL_SetWindowsMessageHook には設計上よろしくない点が多数ある。（結局デバッグ用にウィンドウメッセージの確認を行うしかできない）
	   仕様バグを出しやすくなるので SDL_SetWindowsMessageHook 関数はあまりオススメ出来ないというのが目下の結論
	 */
#if 1
	 // SetWindowSubclass を使う方法
	{ // HWND の取得 http://wiki.libsdl.org/SDL_GetWindowWMInfo
		SDL_SysWMinfo info{};
		SDL_VERSION(&info.version);
		if (SDL_GetWindowWMInfo(window, &info)) {
			assert(IsWindow(info.info.win.window));
			SUBCLASSPROC subclassProc =
				[](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
				switch (uMsg) {
				case WM_IME_STARTCOMPOSITION:
				{
					HIMC hImc = ImmGetContext(reinterpret_cast<HWND>(hwnd));
					if (hImc) {
						LOGFONTW logFont = { 0 };
						logFont.lfHeight = -((LONG)(ImGui::GetFontSize())); // pixel
						logFont.lfWidth = 0;

						wcscpy_s(logFont.lfFaceName, L"ＭＳ ゴシック"); // 初期値は "System" だけれども、これはフォントのサイズが固定になるので、適切な値を与える 
						if (!ImmSetCompositionFontW(hImc, &logFont)) {
							OutputDebugStringA("ImmSetCompositionFontW failed\n");
						}
						ImmReleaseContext(reinterpret_cast<HWND>(hwnd), hImc);
					}
				}
				break;
				case WM_DESTROY:
				{
					if (!RemoveWindowSubclass(hwnd, reinterpret_cast<SUBCLASSPROC>(uIdSubclass), uIdSubclass)) {
						assert(!"RemoveWindowSubclass() failed\n");
					}
				}
				break;
				default:
					break;
				}
				return DefSubclassProc(hwnd, uMsg, wParam, lParam);
			};

			if (!SetWindowSubclass(info.info.win.window, subclassProc, reinterpret_cast<UINT_PTR>(subclassProc), NULL)) {
				assert(!"SetWindowSubclass() failed");
			}
		}
	}
#else
	 // SDL の Windows MessageHook は、ただ一つだけ設定できる
	SDL_SetWindowsMessageHook([](void* userdata, void* hWnd, unsigned int message, Uint64 wParam, Sint64 lParam) {
		if (WM_IME_STARTCOMPOSITION == message) {
			// OutputDebugStringA( "WM_IME_STARTCOMPOSITION\n" );
			HIMC hImc = ImmGetContext(reinterpret_cast<HWND>(hWnd));
			if (hImc) {
				LOGFONTW logFont = { 0 };
				if (ImmGetCompositionFontW(hImc, &logFont)) {
					logFont.lfHeight = -(ImGui::GetFontSize()); // pixel
					logFont.lfWidth = 0;
					wcscpy(logFont.lfFaceName, L"ＭＳ ゴシック"); // 初期値は "System" だけれども、これはフォントのサイズが固定になるので、適切な値を与える 
					if (!ImmSetCompositionFontW(hImc, &logFont)) {
						OutputDebugStringA("ImmSetCompositionFontW Failed \n");
					}
				}
				ImmReleaseContext(reinterpret_cast<HWND>(hWnd), hImc);
			}
		}
		}, nullptr);
#endif

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

	return 0;


	return EXIT_SUCCESS;
}
