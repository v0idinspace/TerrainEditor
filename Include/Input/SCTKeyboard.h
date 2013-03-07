/**
----------------------------------------------------------------------------
	@File:			SCTKeyboard.h
   
	System:         SCTGame Engine 
	Component Name:	SCT Keyboard Interface
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


#ifndef SCT_KEYBOARD_H
#define SCT_KEYBOARD_H


#include <Core\ZephyrHeaders.h>
#include <dinput.h>

namespace Zephyr
{
	namespace Input
	{
		/// Enum of Keyboard Button codes
		//#ifdef SCT_USE_DIRECT_INPUT
		enum SCTKeyboardCode
		{
			SCT_0	= DIK_0,		// On main keyboard
			SCT_1	= DIK_1,		// On main keyboard
			SCT_2	= DIK_2,		// On main keyboard
			SCT_3	= DIK_3,		// On main keyboard
			SCT_4	= DIK_4,		// On main keyboard
			SCT_5	= DIK_5,		// On main keyboard
			SCT_6	= DIK_6,		// On main keyboard
			SCT_7	= DIK_7,		// On main keyboard
			SCT_8	= DIK_8,		// On main keyboard
			SCT_9	= DIK_9,		// On main keyboard
			SCT_A	= DIK_A,
			SCT_PLUS	= DIK_ADD,			// Plus Sign (+) on numeric keypad
					//DIK_APOSTROPHE,
			SCT_B	= DIK_B,
					//DIK_BACK,			// Backspace
					//DIK_BACKSLASH,
			SCT_C	= DIK_C,
					//DIK_CAPITAL,		// Caps Lock
			SCT_COMMA	= DIK_COMMA,
			SCT_D		= DIK_D,
			SCT_DECIMAL	= DIK_DECIMAL,		// Period (decimal point) on numeric keypad
					//DIK_DELETE,
			SCT_DIVIDE	= DIK_DIVIDE,			// Forward slash (/) on numeric keypad
			SCT_DOWN	= DIK_DOWN,			// Down arrow
			SCT_E	= DIK_E,
					//DIK_END,
			SCT_EQUALS = DIK_EQUALS,			// On main keyboard
			SCT_ESC	= DIK_ESCAPE,
			SCT_F	= DIK_F,
					//DIK_F1,
					//DIK_F2,
					//DIK_F3,
					//DIK_F4,
					//DIK_F5,
					//DIK_F6,
					//DIK_F7,
					//DIK_F8,
					//DIK_F9,
					//DIK_F10,
					//DIK_F11,
					//DIK_F12,
			SCT_G	= DIK_G,
					//DIK_GRAVE,			// Grave accent (`)
			SCT_H	= DIK_H,
					//DIK_HOME,
			SCT_I	= DIK_I,
					//DIK_INSERT,
			SCT_J	= DIK_J,
			SCT_K	= DIK_K,
			SCT_L	= DIK_L,
					//DIK_LBRACKET,		// Left square bracket [
			SCT_LCTRL	= DIK_LCONTROL,		// Left Ctrl
			SCT_LEFT	= DIK_LEFT,			// Left arrow
			SCT_LALT	= DIK_LMENU,		// Left Alt
			SCT_LSHIFT	= DIK_LSHIFT,			// Left Shift
					//DIK_LWIN,
			SCT_M	= DIK_M,
			SCT_MINUS	= DIK_MINUS,			// On main keyboard
			SCT_MULTIPLY= DIK_MULTIPLY,		// Asterisk (*) on numeric keypad
			SCT_N	= DIK_N,
					//DIK_NEXT,			// Page down
			SCT_NUMLOCK	= DIK_NUMLOCK,
			SCT_NUM0	= DIK_NUMPAD0,
			SCT_NUM1	= DIK_NUMPAD1,
			SCT_NUM2	= DIK_NUMPAD2,
			SCT_NUM3	= DIK_NUMPAD3,
			SCT_NUM4	= DIK_NUMPAD4,
			SCT_NUM5	= DIK_NUMPAD5,
			SCT_NUM6	= DIK_NUMPAD6,
			SCT_NUM7	= DIK_NUMPAD7,
			SCT_NUM8	= DIK_NUMPAD8,
			SCT_NUM9	= DIK_NUMPAD9,
			SCT_NUMENDER	= DIK_NUMPADENTER,
			SCT_O	= DIK_O,
			SCT_P	= DIK_P,
					//DIK_PAUSE,
			SCT_PERIOD	= DIK_PERIOD,			// On main keyboard
					//DIK_PRIOR,			// Page Up
			SCT_Q	= DIK_Q,
			SCT_R	= DIK_R,
					//DIK_RBRACKET,		// Right square bracket ']'
			SCT_RCTRL	= DIK_RCONTROL,		// Right 'Ctrl'
			SCT_ENTER	= DIK_RETURN,			// 'Enter' on main Keyboard
			SCT_RIGHT	= DIK_RIGHT,			// Right Arrow
			SCT_RALT	= DIK_RMENU,			// Right 'Alt'
			SCT_RSHIFT	= DIK_RSHIFT,			// Right 'Shift'
					//DIK_RWIN,
			SCT_S	= DIK_S,
					//DIK_SEMICOLON,
					//DIK_SLASH,
			SCT_SPACE	= DIK_SPACE,
			SCT_SUBSTRACT	= DIK_SUBTRACT,		// Minus Sign (-) on numeric keypad
					//DIK_SYSRQ,
			SCT_T	= DIK_T,
			SCT_TAB	= DIK_TAB,
			SCT_U	= DIK_U,
			SCT_UP	= DIK_UP,				// Up Arrow
			SCT_V	= DIK_V,
			SCT_W	= DIK_W,
			SCT_X	= DIK_X,
			SCT_Y	= DIK_Y,
			SCT_Z	= DIK_Z,
		};
		//#endif

		/// Interface Keyboard
		//  ...
		class SCTKeyboard
		{
		public:
			virtual ~SCTKeyboard() {};

			virtual void	Initialize() = 0;
			virtual void	Update() = 0;
			virtual void	Shutdown() = 0;

			virtual bool		GetKeyState(SCTKeyboardCode keyID) = 0;
			virtual bool		IsKeyPressed(SCTKeyboardCode keyID) = 0;
			virtual bool		IsKeyReleased(SCTKeyboardCode keyID) = 0;

		protected:

		};

	}	// End of Input Namespace
}	// End of SCT Namespace

#endif