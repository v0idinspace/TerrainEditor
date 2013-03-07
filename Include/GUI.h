/**
----------------------------------------------------------------------------
  File:              GUI.h

  System:            Zephyr Engine
  Component Name:    
  Status:            Version 0.1

  Language:          C++

  License:           

  Author:            Tasos Giannakopoulos (dotvoidd@gmail.com)
  Copyright:         2012 (c) Tasos Giannakopoulos & Matteo Meli
  Date:              30 Nov 2012

  Description:       
----------------------------------------------------------------------------
*/

#ifndef _ZEPHYR_GUI_H_
#define _ZEPHYR_GUI_H_


#include <VoxelHeaders.h>
#include <VoxelTerrainManager.h>


// Handles the gui
class GUI : public Core::Singleton<GUI>
{
public:
  GUI() {}
  GUI(ZUInt32 windowWidth, ZUInt32 windowHeight);
  ~GUI() { Shutdown(); }

  void                    Initialize();
  void                    Update();
  void                    Shutdown();

  // Enable the GUI -- only after the VTM is initilized
  void                    Enabe(bool mode);
  int                     HandleWindowEvents(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  // --- Noise prefab map ---
  NoisePrefabsMapPtr      mNoisePrefabs;

  // --- Terrain parameters ---
  ZUInt32                 mGround;
  float                   mIsoValue;
  ZUInt32                 mTPSize;

  // Height map properties
  bool                    mGenHeightmap;
  ZUInt32                 mHmSampleY;
  ZUInt32                 mHmNumOctaves;
  float                   mHmPersistance;
  float                   mHmIntensity;

  // Noise Layers map
  NoiseLayersMapPtr       mNoiseLayers;
  ZUInt32                 mLayerIndex;

  // Metaballs
  MetaballsMapPtr         mMetaballs;
  ZUInt32                 mMetaballIndex;

  // Height gradient params
  float                   mGradIntensity;
  ZUInt32                 mGradStart;

  ZUInt32                 mHardGroundStart;
  float                   mHardGroundIntensity;
  //\Eof
  
  // --- Stats ---
  float                   mFPS;
  float                   mMsPerFrame;
  ZUInt32                 mCPUUsage;
  ZUInt32                 mNumTriangles;
  char                    mbAutoRotate;
  char                    mbWireframe;
  Math::Vector3           mLightDirection;

  // --- Tw vars ---
  TwType                  mNoiseLayerType;
  TwType                  mNoisePersistanceType;

  TwBar*                  mpTwTerrainDlg;
  TwBar*                  mpTwNoiseDlg; 
  TwBar*                  mpTwStatsDlg;

  bool                    mbIsVTMInitialized; // Checks if the terrain manager is initialized

  // --- Private functions ---
  void                    GenerateNoisePrefabs();
  void                    ClearNoisePrefabs();
  void                    ClearNoiseLayers();
};

// Singleton
template<> GUI* Core::Singleton<GUI>::msSingleton = 0;

#endif // _ZEPHYR_GUI_H_
