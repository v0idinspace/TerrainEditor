/**
----------------------------------------------------------------------------
	@File:			SCTInputManager.h
   
	System:         SCTGame Engine 
	Component Name:	SCT Input Manager (Win32 Implementation)
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


#ifndef SCT_INPUT_MANAGER_WIN32_H
#define SCT_INPUT_MANAGER_WIN32_H


#include <Input\SCTInputManager.h>
#include <dinput.h>


namespace Zephyr
{
	namespace Input
	{
		// Foreward declarations
		class SCTKeyboardWin32;
		class SCTMouseWin32;


		/// Class SCTInputManagerWin32
		//  ...
		class SCTInputManagerWin32 : public SCTInputManager
		{
		public:
			SCTInputManagerWin32(HINSTANCE hInstance, HWND hwnd);
			~SCTInputManagerWin32();

			void				Initialize();
			void				Update();
			void				Shutdown();
      
			void				CreateKeyboardDevice();
			void				CreateMouseDevice();
			
			// Get, Set functions
			inline SCTKeyboard*		GetKeyboard()
			{
				return mpKeyboard;
			}

			inline SCTMouse*		GetMouse()
			{
				return mpMouse;
			}

		protected:

		private:
			// Input Device
			HINSTANCE				mhInstance;
      HWND            mHwnd;
			IDirectInput8			*mpInputDevice;

			SCTKeyboard				*mpKeyboard;
			SCTMouse				*mpMouse;

		};	// End of class SCTInputManager
	}	// End of Input Namespace

}	// End of SCT Namespace

#endif