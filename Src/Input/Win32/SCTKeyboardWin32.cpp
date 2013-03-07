#include <Input\Win32\SCTKeyboardWin32.h>


using namespace Zephyr;
using namespace Input;


SCTKeyboardWin32::SCTKeyboardWin32(IDirectInput8 *inputDevice, HWND hwnd)
:	mpInputDevice (inputDevice),
	mpDXKeyboard (NULL),
  mHwnd(hwnd)
{
}

SCTKeyboardWin32::~SCTKeyboardWin32()
{
	Shutdown();
}

void SCTKeyboardWin32::Initialize()
{
	HRESULT hr;

	// Initialize key button states (none is pressed)
	for(int i = 0; i < 256; i++)
	{
		mButtonState[i]	= false;
    mKeyboardState[i] = false;
	}

	// Initialize the direct input interface for the keyboard.
	hr = mpInputDevice->CreateDevice(GUID_SysKeyboard, &mpDXKeyboard, NULL);
	if(hr != S_OK)
	{
    std::cout<<"$> Failed to create keyboard input device.\n";
	}

	// Set the data format.  In this case since it is a keyboard we can use the predefined data format.
	hr = mpDXKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if(hr != S_OK)
	{
		//return FAIL;
	}

	// Set the cooperative level of the keyboard to not share with other programs.
	hr = mpDXKeyboard->SetCooperativeLevel(mHwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if(hr != S_OK)
	{
		//return FAIL;
	}

	// Now acquire the keyboard.
	hr = mpDXKeyboard->Acquire();
	if(hr != S_OK)
	{
		//return FAIL;
	}

	//return OK;
}

void SCTKeyboardWin32::Update()
{
	if(UpdateKeyboardDevice())
	{

	}

	//return OK;
}

void SCTKeyboardWin32::Shutdown()
{
	// Release the keyboard.
	if(mpDXKeyboard)
	{
		mpDXKeyboard->Unacquire();
    mpDXKeyboard->Release();
		//ReleaseCOM(mpDXKeyboard);
	}

	//return OK;
}

bool SCTKeyboardWin32::GetKeyState(SCTKeyboardCode keyID)
{
	if(mKeyboardState[keyID] & 0x80)
		return true;

	return false;
}

bool SCTKeyboardWin32::IsKeyPressed(SCTKeyboardCode keyID)
{
	if((mKeyboardState[keyID] & 0x80) && !mButtonState[keyID])
	{
		mButtonState[keyID]	= true;
		return true;
	}

	return false;
}

bool SCTKeyboardWin32::IsKeyReleased(SCTKeyboardCode keyID)
{
	if(!(mKeyboardState[keyID] & 0x80) && mButtonState[keyID])
	{
		mButtonState[keyID]	= false;
		return true;
	}

	return false;
}

bool SCTKeyboardWin32::UpdateKeyboardDevice()
{
	HRESULT hr;

	// Read the keyboard device.
	hr = mpDXKeyboard->GetDeviceState(sizeof(mKeyboardState), (LPVOID)&mKeyboardState);
	if(hr != S_OK)
	{
		// If the keyboard lost focus or was not acquired then try to get control back.
		if((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			mpDXKeyboard->Acquire();
		}
		else
		{
			return false;
		}

	}
	
	return true;
}