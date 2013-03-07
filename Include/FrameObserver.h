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


#ifndef _FRAME_OBSERVER_H_
#define _FRAME_OBSERVER_H_


#include <VoxelTerrainManager.h>
#include <OrbitCamera.h>
#include <GUI.h>
#include <Statistics.h>


using namespace Zephyr;


class vtmFrameObserver
{
public:
  vtmFrameObserver();
  ~vtmFrameObserver();

  bool                          NotifyFrameStarted();
  bool                          NotifyFrameEnded();

  void                          Initialize();
  void                          Update();
  void                          HandleInput();
  void                          Shutdown();

  void                          RenderScene(Math::Matrix4 &localToWorldMtx);
  void                          RenderWireframe(Math::Matrix4 &localToWorldMtx);

private:
  /*
  Render::RenderWindow*         mpMainWindow;

  Render::CgShader*             mpVertexShader;
  Render::CgShader*             mpPixelShader;
  Render::CgShader*             mpColorVS;
  Render::CgShader*             mpColorFS;
  */

  // Cgfx
  Render::Effect*               mpLightingFX;
  Render::Effect*               mpColorFX;

  Math::Matrix4                 mWorldMtx, 
                                mViewMtx, 
                                mProjectionMtx;

  // Geometry (PosAmb, Norm, TexCoords01)
  ZUInt32                       mNumVertices;
  ZUInt32                       mPosAoGLBufferID,
                                mNormGLBufferID,
                                mTexCoord01GLBufferID;

  // Scene
  OrbitCamera*                    mpCameraFree;
  float                         mCameraSensitivity;
  float                         mCameraSpeed;

  // Engine related
  String                        mCLProgramsDir,
                                mShadersDir;                      

  // ----- Private functions -----
  void                          InitGPGPU();
  void                          ShutdownGPGPU();
  void                          InitProgram();
  void                          LoadShaders();
  void                          InitGLRenderBuffers();

  // Update stats
  void                          UpdateStats();

};

#endif // _FRAME_OBSERVER_H_