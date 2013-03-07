/**
----------------------------------------------------------------------------
  File:              VoxelTerrainManager.h

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


#ifndef _MARCHING_CUBES_H
#define _MARCHING_CUBES_H


#include <VoxelHeaders.h>


using namespace Zephyr;


/* 
 * Marching cubes
 */
class MarchingCubes : public Core::Singleton<MarchingCubes>
{
public: 
  MarchingCubes();
  ~MarchingCubes();

  void                Initialize(ZUInt32 volumeSize);
  void                Shutdown();

  // Resets the buffers, called after the volume size has changed
  void                ResetBuffers(ZUInt32 volumeSize);

  void                PolygonizeBlock( GPGPU::GPMemory* volumeData,
                                        GPGPU::GPMemory* posBuffer, 
                                        GPGPU::GPMemory* normBuffer,
                                        GPGPU::GPMemory* texCoord01Buffer,
                                        float isoValue,
                                        ZUInt32 maxNumVertices);

  ZUInt32             GetNumVerticesGenerated() { return mNumVertices; }

private:
  // ----- Private variables -----
  ZUInt32             mVolumeSize;
  GPGPU::GPMemory*    mVolumeData;
  ZUInt32             mNumVoxels;
  float               mIsoValue;

  ZUInt32             mMaxNumVertices;
  ZUInt32             mNumVertices;

  // MC program
  GPGPU::GPProgram*   mpProgram;

  // MC Simple version
  GPGPU::GPMemory*    mpNumVertsToGenerate;
  GPGPU::GPMemory*    mTotalNumVertices;
  GPGPU::GPMemory*    mpPosBufferOffset;
  GPGPU::GPMemory*    mpActiveVoxels;
  GPGPU::GPMemory*    mpEdgeTable;
  GPGPU::GPMemory*    mpTriTable;

  // Ptrs to VBOs
  GPGPU::GPMemory*    mpPositionVBO;
  GPGPU::GPMemory*    mpNormalVBO;
  GPGPU::GPMemory*    mpTexCoord01VBO;

  GPGPU::GPMemory*    mpNormalVBO_debug;

  // ----- Private functions -----
  void                InitializeBuffers();
  void                ShutdownBuffers();

  void                ClassifyVoxels();
  void                CalculateVboOffsets();
  void                GenerateTriangles();
  void                ClearBuffers();
};


// Singleton
template<> MarchingCubes* Core::Singleton<MarchingCubes>::msSingleton = 0;

#endif  // _MARCHING_CUBES_H