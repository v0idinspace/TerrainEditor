/**
----------------------------------------------------------------------------
  File:              main.cpp

  System:            Zephyr Engine
  Component Name:    WinMain
  Status:            Version 0.1

  Language:          C++

  License:           

  Author:            Tasos Giannakopoulos (dotvoidd@gmail.com)
  Copyright:         2012 (c) Tasos Giannakopoulos & Matteo Meli
  Date:              22 Jun 2012

  Description:       Voxel terrain (not using ZephyrRenderSystem)

----------------------------------------------------------------------------
*/


#include "FrameObserver.h"


using namespace Zephyr;
using namespace Core;
using namespace Input;
using namespace Render;
using namespace Math;
using namespace GPGPU;


// Forward Declarations 
void InitGL(int argc, char* argv[]);
void InitZephyrModules();
void ShutdownZephyrModules();
LRESULT CALLBACK WndProcCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


// Win32 Variables 
HINSTANCE mHinst;
HWND      mHwnd   = NULL;
HDC       mHDC    = NULL;
HGLRC     mHGLRC  = NULL;

// Render window parameters
ZUInt32 mWinWidth   = 1024;
ZUInt32 mWinHeight  = 640;
LPCWSTR mWinCaption = L"Terrain editor v0.0";

bool mEndLoop = false;


int main(int argc, char* argv[])
{
  // Initialize openGl
  InitGL(0, 0);

  // Initialize zephyr modules
  InitZephyrModules();

  // Initialize the statistics module
  Statistics *stats = new Statistics;
  stats->Initialize();
  stats->EnableFpsStats(true);
  stats->EnableCPUStats(true);

  // Initialize the GUI -- AntTweakBar 
  GUI *gui = new GUI(mWinWidth, mWinHeight);
  GUI::GetSingleton().Initialize();

  // Frame observer
  vtmFrameObserver observer;
  
  // Enter Render loop
  while (!mEndLoop)
  {
    // Message pump
    MSG msg = {0};

    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    // Tick the timer
    Zephyr::Core::Timer::GetSingleton().Tick();

    // Update the statistics
    stats->Update();

    // Render one frame
    if (!observer.NotifyFrameStarted()) // render one frame
      break;

    // Update the GUI
    gui->Update();

    // Swap the uffers
    ::SwapBuffers(mHDC);
  }

  // Clean up
  observer.Shutdown();

  // Shutdown the statistics module
  delete stats;
  stats = NULL;

  // Cleanup gui
  delete gui;
  gui = NULL;

  // Shutdown Zephyr modules
  ShutdownZephyrModules();

  return 0;
}

void InitGL(int argc, char* argv[])
{
  // Create window
  // TODO: This will work only with a static lib compilation
  mHinst = GetModuleHandle(NULL);

  // TODO: Allow customisation through config map.

  // Build up window class and register
  WNDCLASS wc;

  wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc    = WndProcCallback;
  wc.cbClsExtra     = 0;
  wc.cbWndExtra     = 0;
  wc.hInstance      = mHinst;
  wc.hIcon          = LoadIcon(0, IDI_APPLICATION);
  wc.hCursor        = LoadCursor(0, IDC_ARROW);
  wc.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName   = 0;
  wc.lpszClassName  = L"ZephyrRenderWindow";
  RegisterClass(&wc);

  
  // TODO: Change window style based on settings (ie fullscreen, anti aliasing, etc.)

  // Create the render window
  DWORD wndStyle  = WS_OVERLAPPED | WS_CAPTION  | 
                    WS_SYSMENU | WS_MINIMIZEBOX |   // WS_THICKFRAME to allow resize
                    WS_VISIBLE; 

  RECT  winRC     = { 0, 0, mWinWidth, mWinHeight };

  AdjustWindowRect(&winRC, wndStyle, FALSE);
  mHwnd = CreateWindow( L"ZephyrRenderWindow", mWinCaption, 
                        wndStyle, CW_USEDEFAULT, CW_USEDEFAULT, 
                        winRC.right-winRC.left, winRC.bottom-winRC.top, 
                        NULL, NULL, mHinst, NULL);

  ShowWindow(mHwnd, SW_SHOW);      //display window
  UpdateWindow(mHwnd);                     //update window
  
  if (!mHwnd)
    std::cout<<"Error while creating the render window\n";
  //\ Eof Create the window 

  mHDC = GetDC(mHwnd);

  PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize  = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;
 
	int nPixelFormat = ChoosePixelFormat(mHDC, &pfd);
	if (nPixelFormat == 0) 
    std::cout<<"$> ChoosePixelFormat failed.\n";
 
	BOOL bResult = SetPixelFormat (mHDC, nPixelFormat, &pfd);
	if (!bResult)
    std::cout<<"$> SetPixelFormat failed.\n";
 
	HGLRC tempContext = wglCreateContext(mHDC);
	wglMakeCurrent(mHDC, tempContext);
 
	GLenum err = glewInit();
  if(err != GLEW_OK)
    std::cout<<"$> Failed to initialize gl\n";
}

void InitZephyrModules()
{
  // A. Timer
  Core::Timer *mTimer = new Timer();
  mTimer->Initialize();
  mTimer->Reset();

  // B. Filesystem
  Core::FileSystem *fsMngr = new Core::Win32FileSystem();
  fsMngr->Initialize();

  // C. Input manager
  Input::SCTInputManager *inputMngr	= new Input::SCTInputManagerWin32(mHinst, mHwnd);
	inputMngr->Initialize();
  
	inputMngr->CreateKeyboardDevice();
	std::cout<<"$> Keyboard Device has been initialized\n";
  
	inputMngr->CreateMouseDevice();
  std::cout<<"$> Mouse Device has been initialized\n";

  // D. Shader manager
  ShaderManager *mShaderManager = new CgGLShaderManager();
  mShaderManager->SetDebugMode(true); // Always?
  mShaderManager->Initialize();
}

void ShutdownZephyrModules()
{
  // A.  
  ShaderManager *shadermngr = ShaderManager::GetSingletonPtr();
  delete shadermngr;
  shadermngr = NULL;

  // B. Input manager
  Input::SCTInputManager *input = Input::SCTInputManager::GetSingletonPtr();
  delete input;
  input = NULL;

  // C. Filesystem
  Core::FileSystem *filesys = Core::FileSystem::GetSingletonPtr();
  delete filesys ;
  filesys = NULL;

  // D. Timer
  Core::Timer *timer = Core::Timer::GetSingletonPtr();
  delete timer;
  timer = NULL;
}

LRESULT CALLBACK WndProcCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  // Send event message to AntTweakBar
  GUI *gui = GUI::GetSingletonPtr();
  if(gui)
  {
    if(GUI::GetSingleton().HandleWindowEvents(hwnd, message, wParam, lParam))
      return 0;
  }

  // TODO: Should get a reference to the main window created by the rendering system, 
  // cause here is the place where it eventually get destroyed on exit.
  // A pointer to a window has to be stored somewhere and retrived here after the WM_CREATE event.
  if (message == WM_CREATE)
  {
    std::cout<<"Main render window created\n";
    //SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams));
    return 0;
  }

  //Render::RenderWindow* window = (Render::RenderWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  //if (!window)
  //  return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
  {
  case WM_CLOSE:
    //window->Shutdown();
    DestroyWindow(mHwnd);
    PostQuitMessage(0);
    mEndLoop = true;
    return 0;

  case WM_DESTROY:
    DestroyWindow(mHwnd);
    PostQuitMessage(0);
    mEndLoop = true;
    return 0;
  }

  return DefWindowProc(hwnd, message, wParam, lParam);
}