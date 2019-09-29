#include <iostream>
#include <cassert>

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

#if 1
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(lpCmdLine);

	MessageBox(NULL, TEXT("こんにちは"), TEXT("メッセージ"), MB_OK);

	return EXIT_SUCCESS;
}
#else 

int main(int, char[]) {
	std::cout << "hello world" << std::endl;


	return EXIT_SUCCESS;
}
#endif
