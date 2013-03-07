#include <Input\Win32\SCTInputManagerWin32.h>
#include <Input\Win32\SCTKeyboardWin32.h>
#include <Input\Win32\SCTMouseWin32.h>


using namespace Zephyr;
using namespace Input;


SCTInputManagerWin32::SCTInputManagerWin32(HINSTANCE hInstance, HWND hwnd)
:	mhInstance (hInstance),
  mHwnd(hwnd),
	mpInputDevice (NULL),
	mpKeyboard (NULL),
	mpMouse (NULL)
{
	mbIsKeyboard	= false;
	mbIsMouse		= false;
}

SCTInputManagerWin32::~SCTInputManagerWin32()
{
	Shutdown();
}

void SCTInputManagerWin32::Initialize()
{
	HRESULT hr;

	// Initialize the main direct input interface.
	hr = DirectInput8Create(mhInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&mpInputDevice, NULL);
	if(hr != DI_OK)
	{
    std::cout<<"$> Failed to initialize the Input manager\n";
		//Core::SCTLogManager::getSingleton().PrintDebug("SCT Input Manager failed to initialized.");
		//return FAIL;
	}

	//return OK;
}

void SCTInputManagerWin32::Update()
{
	if(mbIsKeyboard)
		mpKeyboard->Update();

	if(mbIsMouse)
		mpMouse->Update();

	//return OK;
}

void SCTInputManagerWin32::Shutdown()
{
	// Shutdown Mouse device
	mbIsMouse	= false;
	delete mpMouse;
	mpMouse	= NULL;

	// Shutdown Keyboard device
	mbIsKeyboard	= false;
	delete mpKeyboard;
	mpKeyboard	= NULL;

	//return OK;
}

void SCTInputManagerWin32::CreateKeyboardDevice()
{
	mpKeyboard	= new SCTKeyboardWin32(mpInputDevice, mHwnd);
	
	mbIsKeyboard	= true;

	mpKeyboard->Initialize();
}

void SCTInputManagerWin32::CreateMouseDevice()
{
	mpMouse	= new SCTMouseWin32(mpInputDevice, mHwnd);
	
	mbIsMouse	= true;

	mpMouse->Initialize();
}