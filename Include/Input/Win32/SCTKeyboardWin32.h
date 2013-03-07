/**
----------------------------------------------------------------------------
	@File:			SCTKeyboardWin32.h
   
	System:         SCTGame Engine 
	Component Name:	SCT Keyboard Win32 Implementation
	Status:         Version 1.0 Release 1 

	Language: C++
   
	License: GNU Public License	[*!]
	Licensed Material - Property of ...
   
	Author: Anastasios 'v0id' Giannakopoulos
	E-Mail: dotvoidd@gmail.com
   
	Description:	Header file for Project X
					This file contains the defined types for Project X
					This is sometimes called the "Abstract" and may be
					followed by a section called "Notes".
 
	Limitations:
   
	Function:

----------------------------------------------------------------------------
*/

#ifndef SCT_KEYBOARD_WIN32_H
#define SCT_KEYBOARD_WIN32_H


#include <Input\SCTKeyboard.h>


namespace Zephyr
{
	namespace Input
	{
		/// Class Interface Keyboard
		//  ...
		class SCTKeyboardWin32 : public SCTKeyboard
		{
		public:
			SCTKeyboardWin32(IDirectInput8 *inputDevice, HWND hwnd);
			~SCTKeyboardWin32();
      
			void				Initialize();
			void				Update();
			void				Shutdown();

			bool					GetKeyState(SCTKeyboardCode keyID);
			bool					IsKeyPressed(SCTKeyboardCode keyID);
			bool					IsKeyReleased(SCTKeyboardCode keyID);

		private:
			// --- Private Variables ---
			IDirectInput8			*mpInputDevice;
      HWND              mHwnd;

			IDirectInputDevice8		*mpDXKeyboard;
			unsigned char			mKeyboardState[256];

			// Holds if the specific key is pressed or released
			bool					mButtonState[256];

			// --- Private Functions ---
			bool				UpdateKeyboardDevice();
		};

		/// Class Interface Keyboard
	}	// End of Input Namespace
}	// End of SCT Namespace

#endif