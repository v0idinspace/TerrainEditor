#include <Input\Win32\SCTMouseWin32.h>


using namespace Zephyr;
using namespace Input;


SCTMouseWin32::SCTMouseWin32(IDirectInput8 *inputDevice, HWND hwnd)
:	mpInputDevice (inputDevice),
  mHwnd(hwnd),
	mpMouse (NULL),
	mScreenWidth (0), 
	mScreenHeight (0),
	mMouseX (0), 
	mMouseY (0)
{
}

SCTMouseWin32::~SCTMouseWin32()
{
	Shutdown();
}

void SCTMouseWin32::Initialize()
{
	HRESULT hr;

	// Initialize mouse button states
	for(int i = 0; i < SCT_NUM_MOUSECODES; i++)
	{
		mButtonState[i]	= false;
	}

	// Initialize the direct input interface for the mouse.
	hr = mpInputDevice->CreateDevice(GUID_SysMouse, &mpMouse, NULL);
	if(hr != S_OK)
	{
		//return FAIL;
	}

	// Set the data format for the mouse using the pre-defined mouse data format.
	hr = mpMouse->SetDataFormat(&c_dfDIMouse);
	if(hr != S_OK)
	{
		//return FAIL;
	}

	// Set the cooperative level of the mouse to share with other programs.
	hr = mpMouse->SetCooperativeLevel(mHwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if(hr != S_OK)
	{
		//return FAIL;
	}

	// Acquire the mouse.
	hr = mpMouse->Acquire();
	if(hr != S_OK)
	{
		//return FAIL;
	}

	// Get Render window size
	//Core::SCTWindows::getSingleton().GetWindowSize(mScreenWidth, mScreenHeight);
	//std::cout<<"Width: " << mScreenWidth << ", " << mScreenHeight << "\n";

	//return OK;
}		  
		  
void SCTMouseWin32::Update()
{
	if(UpdateMouseDevice())
	{
		// Do mouse stuff
		// Update the location of the mouse cursor based on the change of the mouse location during the frame.
		POINT mousePos;

		GetCursorPos(&mousePos);
				
		// Ensure the mouse location doesn't exceed the screen width or height.
		//RECT r;
		//GetWindowRect(Core::SCTWindows::getSingleton().GetRenderWindow(), &r);
    /*
		mMouseX = mousePos.x - r.left;
		mMouseY = mousePos.y - r.top;

		if(mousePos.x < r.left)  
			mMouseX = 0;

		if(mMouseY < r.top)
			mMouseY = 0;
		
		if(mousePos.x > r.right) 
			mMouseX = mScreenWidth;

		if(mMouseY > r.bottom)
			mMouseY = mScreenHeight;
      */
	}

	//return OK;
}		  
		  
void SCTMouseWin32::Shutdown()
{
	// Release the keyboard.
	if(mpMouse)
	{
		mpMouse->Unacquire();
    mpMouse->Release();
		//ReleaseCOM(mpMouse);
	}

	//return OK;
}

bool SCTMouseWin32::IsButtonUp(SCTMouseCode buttonID)
{
	if(!(mMouseState.rgbButtons[buttonID] & 0x80) && mButtonState[buttonID])
	{
		mButtonState[buttonID]	= false;
		return true;
	}

	return false;
}
	  
bool SCTMouseWin32::IsButtonDown(SCTMouseCode buttonID)
{
	if((mMouseState.rgbButtons[buttonID] & 0x80) && !mButtonState[buttonID])
	{
		mButtonState[buttonID]	= true;
		return true;
	}

	return false;
}

bool SCTMouseWin32::GetMouseState(SCTMouseCode buttonID)
{
	if(mMouseState.rgbButtons[buttonID] & 0x80)
		return true;

	return false;
}

bool SCTMouseWin32::UpdateMouseDevice()
{
	HRESULT hr;

	// Read the mouse device.
	hr = mpMouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mMouseState);
	if(hr != S_OK)
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			mpMouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}