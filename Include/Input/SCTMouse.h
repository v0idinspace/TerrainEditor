/**
----------------------------------------------------------------------------
	@File:			SCTMouse.h
   
	System:         SCTGame Engine 
	Component Name:	SCT Mouse Interface
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


#ifndef SCT_MOUSE_H
#define SCT_MOUSE_H


#include <Core\ZephyrHeaders.h>
#include <dinput.h>

namespace Zephyr
{
	namespace Input
	{
		/// Enum of Keyboard Button codes
		//#ifdef SCT_USE_DIRECT_INPUT
		enum SCTMouseCode
		{
			SCT_MOUSE_LEFT	= 0,
			SCT_MOUSE_RIGHT,
			SCT_MOUSE_MIDDLE,
			SCT_NUM_MOUSECODES
		};
		//#endif

		/// Interface Mouse
		//  ...
		class SCTMouse
		{
		public:
			virtual ~SCTMouse() {};

			virtual void	Initialize() = 0;
			virtual void	Update() = 0;
			virtual void	Shutdown() = 0;

			virtual bool		IsButtonUp(SCTMouseCode buttonID) = 0;
			virtual bool		IsButtonDown(SCTMouseCode buttonID) = 0;

			// Get, Set functions
			virtual bool		GetMouseState(SCTMouseCode buttonID) = 0;
			virtual void		GetAbsolutePosition(ZInt32 &x, ZInt32 &y) = 0;
			virtual ZInt32		GetRelativeX() = 0;
			virtual ZInt32		GetRelativeY() = 0;
			virtual ZInt32		GetRelativeZ() = 0;

		protected:

		};

	}	// End of Input Namespace
}	// End of SCT Namespace

#endif