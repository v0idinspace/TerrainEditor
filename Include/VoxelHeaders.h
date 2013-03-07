/**
----------------------------------------------------------------------------
  File:              cgfxFrameObserver.h

  System:            Zephyr Engine
  Component Name:    
  Status:            Version 0.1

  Language:          C++

  License:           

  Author:            Tasos Giannakopoulos (matteo.meli@gmail.com)
  Copyright:         2012 (c) Tasos Giannakopoulos & Matteo Meli
  Date:              01 Aug 2012

  Description:       

----------------------------------------------------------------------------
*/


#ifndef _VOXEL_HEADERS_H_
#define _VOXEL_HEADERS_H_


// Zephyr Headers
#include <Core\Zephyr.h>
#include <Math\ZephyrMath.h>
#include <Render\OpenGL\ZephyrCgGLShaderManager.h>
#include <Core\Win32\ZephyrWin32FileSystem.h>
#include <Render\Cg\ZephyrCgShader.h>
#include <Render\Cg\ZephyrCgShaderManager.h>
#include <GPGPU\ZephyrGPGPUHeaders.h>
#include <GPGPU\ZephyrGPGPUManager.h>

// GUI
#include <AntTweakBar.h>

// Input manager
#include <Input\SCTInputManager.h>
#include <Input\Win32\SCTInputManagerWin32.h>

// Performance Api 
#include <pdh.h>

// Typedefs
struct PositionAmbient
{
public:
  float x, 
        y, 
        z, 
        a;
  
  PositionAmbient() : x(0), y(0), z(0), a(0) {}
  PositionAmbient(float rX, float rY, float rZ, float rA) : x(rX), y(rY), z(rZ), a(rA) {}
};


#endif  // _VOXEL_HEADERS_H_