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


#ifndef _VOXEL_TERRAIN_MANAGER_H
#define _VOXEL_TERRAIN_MANAGER_H


#include <VoxelHeaders.h>
#include <MarchingCubes.h>


using namespace Zephyr;


enum NoiseOctavePersistance
{
  NOP_FULL,     // 1
  NOP_HALF,     // 1/2
  NOP_QUARTER,  // 1/4
  NOP_SQRT2     // 1/sqrt(2)
};

struct NoisePrefab
{
  String            name;
  ZUInt32           index;
  GPGPU::GPMemory*  ptr;
  ZUInt32           size;
  float             frequency;
};

enum NoiseLayerType
{
  NLT_SUB,  // 0
  NLT_ADD   // 1
};

struct NoiseLayerParams
{
  String          name;
  bool            enabled;
  ZUInt32         prefabIndex;
  NoiseLayerType  type;
  float           scale;
  float           intensity;
};

struct Metaball
{
  String        name;
  bool          enabled;
  Math::Vector3 position;
  float         rad,
                i;
};


class Vector8
{
public:
  Vector8(float rS0, float rS1, float rS2, float rS3, float rS4, float rS5, float rS6, float rS7)
    :
  s0(rS0), s1(rS1), s2(rS2), s3(rS3), s4(rS4), s5(rS5), s6(rS6), s7(rS7)
  {}

  float s0, s1, s2, s3, s4, s5, s6, s7;
};

typedef std::map<ZUInt32, NoisePrefab>        NoisePrefabsMap;
typedef std::map<ZUInt32, NoisePrefab*>       NoisePrefabsMapPtr;
typedef std::pair<ZUInt32, NoisePrefab*>      NoisePrefabMapPair;

typedef std::map<String, NoiseLayerParams>    NoiseLayersMap;
typedef std::map<String, NoiseLayerParams*>   NoiseLayersMapPtr;
typedef std::pair<String, NoiseLayerParams>   NoiseLayersMapPair;
typedef std::pair<String, NoiseLayerParams*>  NoiseLayersMapPtrPair;

typedef std::map<String, Metaball>            MetaballsMap;
typedef std::map<String, Metaball*>           MetaballsMapPtr;
typedef std::pair<String, Metaball>           MetaballsMapPair;
typedef std::pair<String, Metaball*>          MetaballsMapPtrPair;

/*
 * Block
 */
class VoxelTerrainManager : public Core::Singleton<VoxelTerrainManager>
{
public:
  VoxelTerrainManager();
  ~VoxelTerrainManager();

  void                Initialize(ZUInt32 posAOGLBuffer, 
                                 ZUInt32 normGLBuffer,
                                 ZUInt32 texCoords01GLBuffer,
                                 ZUInt32 numVertices,
                                 ZUInt32 maxNoisePrefabs);
  void                Export(const char* filename);
  void                Shutdown();


  // Noise Prefabs
  void                AddNoisePrefab(ZUInt32 index, ZUInt32 size, float frequency);
  void                RemoveNoisePrefab(ZUInt32 index);
  void                SetNoisePrefabParams(ZUInt32 index, ZUInt32 size, float frequency);
  void                GenerateNoisePrefab(ZUInt32 index);

  // Terrain
  void                SetTerrainParameters(ZUInt32 size, float isovalue, ZUInt32 ground, float gradIntensity, ZUInt32 gradStart);
  void                GenerateTerrainPatch();
  void                SetTerrainPatchSize(ZUInt32 size) { mTerrainPatchSize = size; }
  ZUInt32             GetTerrainPatchSize() { return mTerrainPatchSize; }

  // Noise Layers
  void                AddNoiseLayer(String &name, bool enabled, NoiseLayerType type, ZUInt32 prefabIndex, float scale, float intensity);
  void                SetNoiseLayerParams(String &name, NoiseLayerType type, bool enabled, ZUInt32 prefabIndex, float scale, float intensity);
  void                RemoveNoiseLayer(String &name);

  // Metaballs
  void                AddMetaball(String &name, bool enabled, Math::Vector3 &position, float radius, float intensity);
  void                SetMetaballParams(String &name, bool enabled, Math::Vector3 &position, float radius, float intensity);
  void                RemoveMetaball(String &name);

  // Other
  void                SetHardGround(ZUInt32 start, float intensity) 
  {
    mHardGroundStart = start; 
    mHardGroundIntensity = intensity;
  }

  void                SertHeightmapParameters(bool generate, ZUInt32 sampleY, ZUInt32 numOctaves, float persistance, float intensity)
  {
    mHMEnable       = (ZUInt32)generate;
    mHMSampleY      = sampleY;
    mHMNumOctaves   = numOctaves;
    mHmPersistance  = persistance;
    mHmIntensity    = intensity;
  }

  ZUInt32             GetCurrentNumVertices() { return mCurrentNumVertices; }

private:
  // ----- Private variables -----
  GPGPU::GPProgram*   mpNoiseProg;
  GPGPU::GPProgram*   mpTerrainProg;

  // Shared geometry buffers
  ZUInt32             mVertexBufferMaxSize;
  ZUInt32             mCurrentNumVertices;

  GPGPU::GPMemory     *mPosBuffer, 
                      *mNormBuffer,
                      *mTexCoord01Buffer;

  // --- Noise prefabs ---
  ZUInt32             mMaxNoisePrefabs;
  NoisePrefabsMapPtr  mNoisePrefabs;

  // --- Terrain ---
  ZUInt32             mTerrainPatchSize;
  float               mIsoValue;
  ZUInt32             mGround;
  GPGPU::GPMemory*    mpTerrainVolumeWrite;
  GPGPU::GPMemory*    mpTerrainVolumeRead;

  // Heightmap
  ZUInt32             mHMEnable;
  ZUInt32             mHMSampleY;
  ZUInt32             mHMNumOctaves;
  float               mHmPersistance;
  float               mHmIntensity;

  // Noise layers
  ZUInt32             mMaxNoiseLayers;
  NoiseLayersMap      mNoiseLayers;
  GPGPU::GPMemory*    mpNoiseLayersBuffer;

  // Metaballs
  MetaballsMap        mMetaballs;
  GPGPU::GPMemory*    mpMetaballsBuffer;

  // Height gradient params
  float               mGradIntensity;
  ZUInt32             mGradStart;

  // Hard ground params
  ZUInt32             mHardGroundStart;
  float               mHardGroundIntensity;

  // ----- Private functions -----
  // Noise functions
  void                GenerateTerrainVolume();
  void                GenerateNoisePrefabVolume(NoisePrefab *prefab);

  // GP Buffers
  void                InitializeBuffers(ZUInt32 posAOGLBuffer, ZUInt32 normGLBuffer, ZUInt32 texCoords01GLBuffer);
  void                ShutdownBuffers();

  // Volumes
  void                ClearNoisePrefabs();
  void                ClearTerrainVolume();
};

// Singleton
template<> VoxelTerrainManager* Core::Singleton<VoxelTerrainManager>::msSingleton = 0;

#endif // _VOXEL_TERRAIN_MANAGER_H
