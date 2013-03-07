/**
----------------------------------------------------------------------------
	@File:			SCTMouseWin32.h
   
	System:         SCTGame Engine 
	Component Name:	SCT Mouse Win32 Implementation
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

#ifndef SCT_MOUSE_WIN32_H
#define SCT_MOUSE_WIN32_H


#include <Input\SCTMouse.h>


namespace Zephyr
{
	namespace Input
	{
		/// Interface Mouse
		//  ...
		class SCTMouseWin32 : public SCTMouse
		{
		public:
			SCTMouseWin32(IDirectInput8 *inputDevice, HWND hwnd);
			~SCTMouseWin32();

			void				Initialize();
			void				Update();
			void				Shutdown();

			bool					IsButtonUp(SCTMouseCode buttonID);
			bool					IsButtonDown(SCTMouseCode buttonID);

			// Get, Set functions
			bool					GetMouseState(SCTMouseCode buttonID);

			inline void				GetAbsolutePosition(ZInt32 &x, ZInt32 &y)
			{
				x	= mMouseX;
				y	= mMouseY;
			}

			inline ZInt32			GetRelativeX()
			{
				return (ZInt32)mMouseState.lX;
			}

			inline ZInt32			GetRelativeY()
			{
				return (ZInt32)mMouseState.lY;
			}

			inline ZInt32			GetRelativeZ()
			{
				return (ZInt32)mMouseState.lZ;
			}

		private:
			// --- Private Variables ---
			IDirectInput8			*mpInputDevice;
      HWND              mHwnd;

			IDirectInputDevice8		*mpMouse;
			DIMOUSESTATE			mMouseState;
			
			// Mouse button states
			bool					mButtonState[SCT_NUM_MOUSECODES];

			ZInt32					mScreenWidth, 
									mScreenHeight,
									mMouseX, 
									mMouseY;

			// --- Private functions ---
			bool				UpdateMouseDevice();

		};

	}	// End of Input Namespace
}	// End of SCT Namespace

#endif