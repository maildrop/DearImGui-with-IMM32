#include <iostream>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_opengl.h>

#include <Windows.h>
#include <commctrl.h>

#include "glyph-ranges-ja.h"

#include "TextEditor.h"

#if defined( _WIN32 )
#include "imgui_imm32_onthespot.h"
#include "imgui_imm32_onthespot_sdl.h"
#endif /* defined( _WIN32 ) */

#include <locale>

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


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
  UNREFERENCED_PARAMETER(lpCmdLine);

  /*********************************************/
  /* IMM32 変更点  #1                            */
  /*********************************************/

  // snprintf 等が wide-multibyte 変換で使うのでローケルの設定は必要
  // ただし windows では mingw の gcc は、ローケルが Cローケルしか受け付けないのでこれは、Visual C++ ぐらいしか使えない
  // (mingw の 仕様のミスなので、当方では修正できません）
  (void)std::locale::global( std::locale("") ); 
    
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
  constexpr Uint32 window_flags = 
    imgex::composite_flags<Uint32>(SDL_WINDOW_OPENGL,SDL_WINDOW_RESIZABLE, SDL_WINDOW_ALLOW_HIGHDPI);
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
      config.SizePixels = 16.0f * 1.0f;
      return config;
    }();

    io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF("../IMM32IMGUI/NotoSansMonoCJKjp-Regular.otf", 
                                 16.0f,
                                 &config, 
                                 getJapaneseGlyphRanges()); //  io.Fonts->GetGlyphRangesJapanese());
  }

  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
  //IM_ASSERT(font != NULL);

  // Our state

  /*********************************************/
  /* IMM32 変更点  #2                            */
  /*********************************************/
  ImGUIIMMCommunication imguiIMMCommunication{};
  VERIFY( imguiIMMCommunication.subclassify( window ) );


  TextEditor editor{};
  
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  // Main loop
  for(;;){
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    SDL_Event event;
    while (SDL_PollEvent(&event)){
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT){
        goto END_OF_MAIN_LOOP;
      }
    }
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
    
    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
      ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
      ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
      ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
      ImGui::SameLine();
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();
    }
    

    {

      if (ImGui::Begin("Input Test")) {

        static char buffer1[1024] = { 0 };
        static char buffer2[1024] = { 0 };
        ImGui::InputText("input1", buffer1, std::extent<decltype(buffer1)>::value);
        ImGui::InputText("input2", buffer2, std::extent<decltype(buffer2)>::value);
        //ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
      }
      ImGui::End();

      static ImGuiID lastTextInputFocusId = 0;
      static ImGuiID lastTextInputNavId = 0;
      static ImGuiWindow* lastTextInputNavWindow = nullptr;
      {
        ImGuiContext& g = *GImGui;
        if ((g.WantTextInputNextFrame != -1) ? (g.WantTextInputNextFrame != 0) : false) {
          // マウスクリックしてる間は、ActiveID が切り替わるので、
          if (!ImGui::IsMouseClicked(0)) { // この条件アドホック過ぎ
            lastTextInputNavWindow = ImGui::GetCurrentContext()->NavWindow;
            lastTextInputFocusId = ImGui::GetActiveID();
            lastTextInputNavId = ImGui::GetFocusID();
          }
        }
      }

      // ごめん手段が思いつかない。
      // 閉じる要件として、 IsRootwindowOrAnyChildFocused() が false の時っていうのがあるね。

      if (ImGui::Begin("number2", nullptr, 
                       ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_NoTitleBar| 
                       ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_AlwaysAutoResize |
                       ImGuiWindowFlags_NoSavedSettings |
                       ImGuiWindowFlags_NoFocusOnAppearing|                 
                       ImGuiWindowFlags_NoBringToFrontOnFocus |
                       ImGuiWindowFlags_NoNavInputs|
                       ImGuiWindowFlags_NoNavFocus )) {
 
        ImGui::Text("Active id : %u , NavWindow : %x %d", ImGui::GetActiveID() , ImGui::GetCurrentContext()->NavWindow , ImGui::GetIO().WantTextInput);
        ImGui::Text("last Active:%u , NavWindow : %x ", lastTextInputFocusId, lastTextInputNavWindow);
        if (ImGui::ButtonEx("popup",ImVec2(0.0f,0.0f), ImGuiButtonFlags_NoNavFocus)) {
          /*
            if (ImGui::IsRootWindowOrAnyChildFocused()) {
            OutputDebugStringW(L"IsRootWindowOrAnyChildFocused()");
            } else {
            OutputDebugStringW(L"! IsRootWindowOrAnyChildFocused()");
            }
          */
          if (ImGui::IsWindowFocused (ImGuiFocusedFlags_RootAndChildWindows) &&
              !ImGui::IsAnyItemActive() &&
              !ImGui::IsMouseClicked(0))
            {
              if (lastTextInputFocusId && lastTextInputNavId) {
                ImGui::SetActiveID(lastTextInputFocusId, lastTextInputNavWindow);
                ImGui::SetFocusID(lastTextInputNavId, lastTextInputNavWindow);
              }
              // ImGui::SetKeyboardFocusHere(-1);
            }
                

          //    ImGui::SetKeyboardFocusHere(0);
          OutputDebugStringW(L"popup\n");
        }

        ImGui::SameLine();
        ImGui::Text("%s", ImGui::IsWindowFocused() ? u8"フォーカスを持ってる" : u8"フォーカスが無い");


        if (ImGui::Button("hoge")) {


        }
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
            


      }
      ImGui::End();
    }

    auto cpos = editor.GetCursorPosition();
    ImGui::Begin("Text Editor Demo", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
    ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::BeginMenuBar())
      {
        if (ImGui::BeginMenu("File"))
          {
            if (ImGui::MenuItem("Save"))
              {
                auto textToSave = editor.GetText();
                /// save text....
              }
            if (ImGui::MenuItem("Quit", "Alt-F4"))
              break;
            ImGui::EndMenu();
          }
        if (ImGui::BeginMenu("Edit"))
          {
            bool ro = editor.IsReadOnly();
            if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
              editor.SetReadOnly(ro);
            ImGui::Separator();

            if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
              editor.Undo();
            if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
              editor.Redo();

            ImGui::Separator();

            if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
              editor.Copy();
            if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
              editor.Cut();
            if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
              editor.Delete();
            if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
              editor.Paste();

            ImGui::Separator();

            if (ImGui::MenuItem("Select all", nullptr, nullptr))
              editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

            ImGui::EndMenu();
          }

        if (ImGui::BeginMenu("View"))
          {
            if (ImGui::MenuItem("Dark palette"))
              editor.SetPalette(TextEditor::GetDarkPalette());
            if (ImGui::MenuItem("Light palette"))
              editor.SetPalette(TextEditor::GetLightPalette());
            if (ImGui::MenuItem("Retro blue palette"))
              editor.SetPalette(TextEditor::GetRetroBluePalette());
            ImGui::EndMenu();
          }
        ImGui::EndMenuBar();
      }
    /*
      ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
      editor.IsOverwrite() ? "Ovr" : "Ins",
      editor.CanUndo() ? "*" : " ",
      editor.GetLanguageDefinition().mName.c_str(), fileToEdit);
    */
                
    editor.Render("TextEditor");
    ImGui::End();

    /*********************************************/
    /* IMM32 変更点  #3                            */
    /*********************************************/
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
 END_OF_MAIN_LOOP:

  // Cleanup
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  
  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}
