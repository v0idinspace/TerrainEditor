/**
----------------------------------------------------------------------------
  File:              MCHelper.h

  System:            Zephyr Engine
  Component Name:    
  Status:            Version 0.1

  Language:          C++

  License:           

  Author:            Tasos Giannakopoulos (dotvoidd@gmail.com)
  Copyright:         2012 (c) Tasos Giannakopoulos & Matteo Meli
  Date:              19 Nov 2012

  Description:       
----------------------------------------------------------------------------
*/


#ifndef _MC_HELPER_H_
#define _MC_HELPER_H_


#include <VoxelHeaders.h>
#include <Core\ZephyrSingleton.h>


using namespace Zephyr;


class VoxelHelper : public Core::Singleton<VoxelHelper>
{
public:
  VoxelHelper();
  ~VoxelHelper();

  void              Initialize();
  void              Shutdown();

  size_t            RoundUp(int groupSize, int globalSize);

private:
  // ----- Private variables -----
  ZUInt32           mWorkGroupSize;

  GPGPU::GPProgram* mpProgram;
  GPGPU::GPMemory*  mpScanBuffer;

  // ----- Private functions -----

};

// Singleton
template<> VoxelHelper* Core::Singleton<VoxelHelper>::msSingleton = 0;

#endif  // _MC_HELPER_H_