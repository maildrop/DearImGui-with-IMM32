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

/** �R�����R���g���[���̏����� */
static int common_control_initialize()
{
	// 
	// �� InitCommonControls() ���Ăяo���Ă���̂ŁA Comctl32.dll �͈ÖٓI�Ƀ����N���Ă���
	// �������Ȃ���A ���߂��Ă���̂� InitCommonControlsEx �ł���̂ŁA�����炪���݂��Ă���΂�����g�� 
	HMODULE comctl32 = nullptr;
	if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, L"Comctl32.dll", &comctl32)) {
		return EXIT_FAILURE;
	}

	assert(comctl32 != nullptr);
	if (comctl32) {
		{ // InitCommonControlsEx ���g�p���ď����������݂�
			typename std::add_pointer< decltype(InitCommonControlsEx) >::type lpfnInitCommonControlsEx =
				reinterpret_cast<typename std::add_pointer< decltype(InitCommonControlsEx) >::type>(GetProcAddress(comctl32, "InitCommonControlsEx"));
			// InitCommonControlsEx �����������ꍇ
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
		{ //InitCommonControls ���g�p���ď����������݂�
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

	MessageBox(NULL, TEXT("����ɂ���"), TEXT("���b�Z�[�W"), MB_OK);

	return EXIT_SUCCESS;
}
#else 

int main(int, char[]) {
	std::cout << "hello world" << std::endl;


	return EXIT_SUCCESS;
}
#endif
