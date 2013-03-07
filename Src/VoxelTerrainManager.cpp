#include <VoxelTerrainManager.h>
#include <VoxelHelper.h>


using namespace Zephyr;
using namespace GPGPU;


VoxelTerrainManager::VoxelTerrainManager()
: mpNoiseProg(NULL),
  mpTerrainProg(NULL),
  mPosBuffer(NULL), 
  mNormBuffer(NULL),
  mTexCoord01Buffer(NULL),
  mpTerrainVolumeWrite(NULL),
  mpTerrainVolumeRead(NULL),
  mpNoiseLayersBuffer(NULL),
  mpMetaballsBuffer(NULL),
  mTerrainPatchSize(64),
  mIsoValue(0),
  mHMEnable(false),
  mHMSampleY(0),
  mHMNumOctaves(1),
  mHmPersistance(1.0f),
  mHmIntensity(1.0f),
  mGradIntensity(1.0f),
  mGradStart(0),
  mHardGroundStart(0),
  mHardGroundIntensity(1.0f),
  mCurrentNumVertices(0)
{  
  // Initialize the voxel helper
  VoxelHelper *voxelHelper  = new VoxelHelper;
  MarchingCubes *mc         = new MarchingCubes();  // TODO: dynamicaly change size (8, 16, 32) +1
  
  // Load the programs
  String programData = Core::FileSystem::GetSingleton().GetFileData("CLPrograms\\noise.cl", Core::FF_TEXT);
  mpNoiseProg = GPGPUManager::GetSingleton().LoadProgram("Noise", programData);
  
  programData = Core::FileSystem::GetSingleton().GetFileData("CLPrograms\\terrain.cl", Core::FF_TEXT);
  mpTerrainProg = GPGPUManager::GetSingleton().LoadProgram("Terrain", programData);
}

VoxelTerrainManager::~VoxelTerrainManager()
{
  Shutdown();
}

void VoxelTerrainManager::Initialize(ZUInt32 posAOGLBuffer, ZUInt32 normGLBuffer, ZUInt32 texCoords01GLBuffer, ZUInt32 numVertices, ZUInt32 maxNoisePrefabs)
{
  mVertexBufferMaxSize  = numVertices;
  mMaxNoisePrefabs      = maxNoisePrefabs;

  // Initialize the buffers
  InitializeBuffers(posAOGLBuffer, normGLBuffer, texCoords01GLBuffer);

  // Initialize Marching cubes for terrain patch
  MarchingCubes::GetSingleton().Initialize(mTerrainPatchSize);
}

void VoxelTerrainManager::Export(const char* filename)
{
  std::ostringstream outData;

  // Noise prefabs
  outData << "-------------------------------------\n";
  outData << "# Noise prefabs\n";

  NoisePrefabsMapPtr::iterator  itNP;
  for(itNP = mNoisePrefabs.begin(); itNP != mNoisePrefabs.end(); itNP++)
  {
    NoisePrefab *np = (*itNP).second;
    outData << "Noise prefab:\t"  << np->index << "\n";
    outData << "Size:\t\t\t" << np->size << "\n";
    outData << "Frequency:\t\t" << np->frequency << "\n";
  }

  // Global params
  outData << "-------------------------------------\n";
  outData << "# Terrain patch global parameters\n";
  outData << "Size:\t\t\t"    << mTerrainPatchSize << "\n";
  outData << "Iso-value:\t\t" << mIsoValue << "\n";
  outData << "Ground:\t\t\t"  << mGround << "\n";  

  // Metaballs
  outData << "-------------------------------------\n";
  outData << "# Metaballs\n";

  MetaballsMap::iterator itM;
  for(itM = mMetaballs.begin(); itM != mMetaballs.end(); itM++)
  {
    Metaball &mb = (*itM).second;
    outData << "Metaball:\t\t" << mb.name << " ";
    if(mb.enabled)
      outData << "enabled\n";
    else
      outData << "disabled\n";

    outData << "Position:\t\tx "  << mb.position.x << " y " << mb.position.y << " z " << mb.position.z << "\n";
    outData << "Radius:\t\t\t"      << mb.rad << "\n";
    outData << "Intensity:\t\t"   << mb.i << "\n";
  }

  // NoiseLayers
  outData << "-------------------------------------\n";
  outData << "# Noise layers\n";

  NoiseLayersMap::iterator itNL;
  for(itNL = mNoiseLayers.begin(); itNL != mNoiseLayers.end(); itNL++)
  {
    NoiseLayerParams &nl = (*itNL).second;
    
    outData << "Noise layer:\t" << nl.name << " ";
    if(nl.enabled)
      outData << "enabled\n";
    else
      outData << "disabled\n";

    outData << "Prefab index:\t"<< nl.prefabIndex << "\n";
    outData << "Type:\t\t\t";
    if(nl.type == NLT_ADD)
      outData << "ADD\n";
    else if(nl.type == NLT_SUB)
      outData << "SUB\n";

    outData << "Scale:\t\t\t"<< nl.scale << "\n";
    outData << "Intensity:\t\t"<< nl.intensity << "\n";
  }
    
  // HG, HG
  outData << "-------------------------------------\n";
  outData << "# Height gradient\n";
  outData << "Start height:\t"  << mGradStart << "\n";
  outData << "Intensity:\t\t"     << mGradIntensity << "\n";
  
  outData << "-------------------------------------\n";
  outData << "# Hard ground\n";
  outData << "Start height:\t"  << mHardGroundStart << "\n";
  outData << "Intensity:\t\t"     << mHardGroundIntensity << "\n";

  Core::FileSystem::GetSingleton().WriteDataToFile(filename, outData.str());
}

void VoxelTerrainManager::Shutdown()
{
  // Clear the prefabs
  ClearNoisePrefabs();

  // Clear terrain volumes
  //ShutdownTerrainVolume();

  // Shutdown the buffers
  //ShutdownBuffers();

  // Marching cubes
  MarchingCubes *mc = MarchingCubes::GetSingletonPtr();
  delete mc;
  mc = NULL;

  // Voxel Helper
  VoxelHelper *vh = VoxelHelper::GetSingletonPtr();
  delete vh;
  vh = NULL;

  //std::cout<<"";
}

// Noise prefabs
void VoxelTerrainManager::AddNoisePrefab(ZUInt32 index, ZUInt32 size, float frequency)
{
  NoisePrefab *prefab = new NoisePrefab;

  std::ostringstream ssName;
  ssName << "NoisePrefab" << index;

  prefab->name       = ssName.str();
  prefab->index      = index;
  prefab->size       = size;
  prefab->frequency  = frequency;
  prefab->ptr        = NULL;

  // Add the noise to the map
  mNoisePrefabs.insert(NoisePrefabMapPair(index, prefab));

  // Now update the noise texture
  GenerateNoisePrefabVolume(prefab);
}

void VoxelTerrainManager::RemoveNoisePrefab(ZUInt32 index)
{
  // Find the noise in the map
  NoisePrefabsMapPtr::iterator itNP = mNoisePrefabs.find(index);

  if(itNP != mNoisePrefabs.end())
  {
    NoisePrefab *prefabParams = (*itNP).second;

    if(prefabParams)
    {
      // Release the memory object that holds the noise data
      GPGPUManager::GetSingleton().ReleaseMemObject(prefabParams->name.c_str());
      delete prefabParams;
      prefabParams = NULL;
    }

    // Remove from map
    mNoisePrefabs.erase(index);
  }
}

void VoxelTerrainManager::SetNoisePrefabParams(ZUInt32 index, ZUInt32 size, float frequency)
{
  // Find the noise in the map
  NoisePrefabsMapPtr::iterator itNP = mNoisePrefabs.find(index);

  if(itNP != mNoisePrefabs.end())
  {
    NoisePrefab *prefabParams = (*itNP).second;

    prefabParams->size       = size;
    prefabParams->frequency  = frequency;

    // Now update the noise texture
    GenerateNoisePrefabVolume(prefabParams);
  }
  else
  {
    // Error -- No noise with that name
    std::cout<< "$> Noise prefab '" << index << "'  could not be found\n";
  }
}

// Terrain 
void VoxelTerrainManager::SetTerrainParameters(ZUInt32 size, float isovalue, ZUInt32 ground, float gradIntensity, ZUInt32 gradStart)
{
  // Check if the terrain patch size changed
  if(size != mTerrainPatchSize)
  {
    mTerrainPatchSize = size;

    // Reset MC module for new volume size
    MarchingCubes::GetSingleton().ResetBuffers(size);

    // Update the terrain
    GenerateTerrainVolume();
  }

  mIsoValue         = isovalue;
  mGround           = ground;
  mGradIntensity    = gradIntensity;
  mGradStart        = gradStart; 
}

// Noise layers
void VoxelTerrainManager::AddNoiseLayer(String &name, bool enabled, NoiseLayerType type, ZUInt32 prefabIndex, float scale, float intensity)
{
  NoiseLayerParams nLayer;
  nLayer.name         = name;
  nLayer.type         = type;
  nLayer.enabled      = enabled;
  nLayer.prefabIndex  = prefabIndex;
  nLayer.scale        = scale;
  nLayer.intensity    = intensity;
  
  mNoiseLayers.insert(NoiseLayersMapPair(name, nLayer));
}

void VoxelTerrainManager::RemoveNoiseLayer(String &name)
{
  // Find the layer
  NoiseLayersMap::iterator itNL;

  itNL = mNoiseLayers.find(name);
  if(itNL != mNoiseLayers.end())
    mNoiseLayers.erase(name);
}

void VoxelTerrainManager::SetNoiseLayerParams(String &name, NoiseLayerType type, bool enabled, ZUInt32 prefabIndex, float scale, float intensity)
{
  // Find the layer
  NoiseLayersMap::iterator itNL;

  itNL = mNoiseLayers.find(name);
  if(itNL != mNoiseLayers.end())
  {
    (*itNL).second.type         = type;
    (*itNL).second.enabled      = enabled;
    (*itNL).second.prefabIndex  = prefabIndex;
    (*itNL).second.scale        = scale;
    (*itNL).second.intensity    = intensity;
  }
}

void VoxelTerrainManager::AddMetaball(String &name, bool enabled, Math::Vector3 &position, float radius, float intensity)
{
  Metaball metaball;
  metaball.name     = name;
  metaball.enabled  = enabled;
  metaball.position = position;
  metaball.rad      = radius;
  metaball.i        = intensity;

  mMetaballs.insert(MetaballsMapPair(name, metaball));
}

void VoxelTerrainManager::SetMetaballParams(String &name, bool enabled, Math::Vector3 &position, float radius, float intensity)
{
  // Find the metaball
  MetaballsMap::iterator itM;

  itM = mMetaballs.find(name);
  if(itM != mMetaballs.end())
  {
    (*itM).second.enabled   = enabled;
    (*itM).second.position  = position;
    (*itM).second.rad       = radius;
    (*itM).second.i         = intensity;
  }
}

void VoxelTerrainManager::RemoveMetaball(String &name)
{
  // Find the metaball
  MetaballsMap::iterator itM;

  itM = mMetaballs.find(name);
  if(itM != mMetaballs.end())
    mMetaballs.erase(name);
}

// Private functions
// Generates a Noise volume 
void VoxelTerrainManager::GenerateNoisePrefabVolume(NoisePrefab *prefab)
{
  std::ostringstream writeGpName, readGpName;
  writeGpName << prefab->name << "Write";
  readGpName << prefab->name << "Read";

  ZUInt32 size = prefab->size;
  float   freq = prefab->frequency;

  // Get the noise params based on the noise index
  GPMemory *noiseTexture_Write  = NULL;
  GPMemory *noiseTexture_Read   = NULL;

  // Release the current noise (Read)
  if(prefab->ptr)
  {
    GPGPUManager::GetSingleton().ReleaseMemObject(readGpName.str().c_str());
    prefab->ptr = NULL;
  }

  size_t noiseTexOrigin[] = { 0, 0, 0, 0  };
  size_t noiseTexRegion[] = { size, size, size, 0};

  // Create Write image
  noiseTexture_Write = GPGPUManager::GetSingleton().CreateImage(
                        writeGpName.str().c_str(), 
                        MA_WRITE_ONLY, MF_NONE,
                        MOT_IMAGE3D, ICO_R, ICDT_FLOAT, 
                        size, size, size, NULL);

  // Global work size (gws) of noise kernel
  GPKernel *krnlGenNoise = mpNoiseProg->GetKernel("GradientNoiseImage3D");

  // Work group, etc
  ZUInt32 workDimension     = 2;
  size_t  lws[2]  = { 16, 16 };
  size_t  gws[2]  = { VoxelHelper::GetSingleton().RoundUp(lws[0], size),
                      VoxelHelper::GetSingleton().RoundUp(lws[1], size) };

  // Set the kernel arguments
  krnlGenNoise->SetArgument(0, noiseTexture_Write);
  krnlGenNoise->SetArgument(1, &freq);
  krnlGenNoise->SetArgument(2, &size);

  // Execute the kernel
  GPGPUManager::GetSingleton().ExecuteKernel(krnlGenNoise, workDimension, NULL, gws, lws);


  std::vector<float> noiseBufferVec(size*size*size);
  GPGPUManager::GetSingleton().ReadImage(noiseTexture_Write, noiseTexOrigin, noiseTexRegion, &noiseBufferVec[0], true);
  
  noiseTexture_Read = GPGPUManager::GetSingleton().CreateImage(
                        readGpName.str().c_str(),
                        MA_READ_ONLY, MF_COPY_HOST_PTR, 
                        MOT_IMAGE3D, ICO_R, ICDT_FLOAT,
                        size, size, size, 
                        &noiseBufferVec[0]);

  // Release the write image
  GPGPUManager *gpgpu = GPGPUManager::GetSingletonPtr();
  GPGPUManager::GetSingleton().ReleaseMemObject(writeGpName.str().c_str());

  // Update the noise ptr
  prefab->ptr = noiseTexture_Read;
}

/// Generates a terrain patch 
void VoxelTerrainManager::GenerateTerrainPatch()
{
  // Generate the terrain volume
  GenerateTerrainVolume();

  // Clear the VBO
  GPKernel *krnlClearVBO = mpTerrainProg->GetKernel("ClearVBO");
  krnlClearVBO->SetArgument(0, mPosBuffer);
  krnlClearVBO->SetArgument(1, mNormBuffer);
  krnlClearVBO->SetArgument(2, &mVertexBufferMaxSize);

  ZUInt32 tWorkDimension     = 1;
  size_t  lws[3]  = { 1, 0 };
  size_t  gws[3] = {VoxelHelper::GetSingleton().RoundUp(lws[0], mVertexBufferMaxSize), 0};
  GPMemory* interopBuffers[] = { mPosBuffer, mNormBuffer };

  // Acquire the GL buffers
  GPGPUManager::GetSingleton().AcquireHardwareBuffer(1, interopBuffers);
  GPGPUManager::GetSingleton().ExecuteKernel(krnlClearVBO, tWorkDimension, NULL, gws, lws);

  // Release the GL buffers
  GPGPUManager::GetSingleton().ReleaseHardwareBuffer(1, interopBuffers);
  //\ Eof clear VBO
  
  // Polygonize it
  MarchingCubes::GetSingleton().PolygonizeBlock(
    mpTerrainVolumeRead, 
    mPosBuffer, mNormBuffer, mTexCoord01Buffer, 
    mIsoValue, 
    mVertexBufferMaxSize);

  // Update the number of vertices generated
  mCurrentNumVertices = MarchingCubes::GetSingleton().GetNumVerticesGenerated();
}

/// Generates the general terrain structure (terrain patch of 256^3 size)
void VoxelTerrainManager::GenerateTerrainVolume()
{
  // Release the Read volume
  GPGPUManager::GetSingleton().ReleaseMemObject("TerrainPatchVolume_Read");
  mpTerrainVolumeRead = NULL;

  // Ground...
  if(mGround > mTerrainPatchSize)
    mGround = mTerrainPatchSize - 1;

  size_t terrainGridOrigin[] = {0, 0, 0, 0};
  size_t terrainGridRegion[] = {mTerrainPatchSize, mTerrainPatchSize, mTerrainPatchSize, 0};

  // Create the terrain write volume
  mpTerrainVolumeWrite  = GPGPUManager::GetSingleton().CreateImage(
                            "TerrainPatchVolume_Write",
                            MA_WRITE_ONLY, MF_NONE, 
                            MOT_IMAGE3D, ICO_R, ICDT_FLOAT,
                            mTerrainPatchSize, mTerrainPatchSize, mTerrainPatchSize,
                            NULL);

  // Format the Noise layer buffer
  ZUInt32 numNoiseLayers = mNoiseLayers.size();
  std::vector<Math::Vector4> noiseLayers;

  if(!numNoiseLayers)
    noiseLayers.push_back(Math::Vector4(0, 0, 0, 0));

  std::map<String, NoiseLayerParams>::iterator itNL;
  for(itNL = mNoiseLayers.begin(); itNL != mNoiseLayers.end(); itNL++)
  {
    NoiseLayerParams &params = (*itNL).second;

    if(params.enabled)
    {
      // PrefabIndex | Scale | Intensity
      Math::Vector4 nl( (float)params.prefabIndex,
                        (float)params.type,
                        (params.scale * 32.0f)/64.0f, 
                        params.intensity);

      noiseLayers.push_back(nl);
    }
  }

  numNoiseLayers = noiseLayers.size();

  if(numNoiseLayers)
    GPGPUManager::GetSingleton().WriteBuffer(mpNoiseLayersBuffer, 0, numNoiseLayers, &noiseLayers[0]);
  //\ Eof Noise layers buffer

  // Format the metaballs buffer
  ZUInt32 numMetaballs = mMetaballs.size();
  std::vector<Vector8> metaballs;

  if(!numMetaballs)
    metaballs.push_back(Vector8(0, 0, 0, 0, 0, 0, 0, 0));

  MetaballsMap::iterator itM;
  for(itM = mMetaballs.begin(); itM != mMetaballs.end(); itM++)
  {
    Metaball &metaball = (*itM).second;

    if(metaball.enabled)
    {
      Vector8 mbParams = Vector8( metaball.position.x, metaball.position.y, metaball.position.z, 
                                  metaball.rad, metaball.i, 0, 0, 0);

      metaballs.push_back(mbParams);
    }
  }

  numMetaballs = metaballs.size();

  std::cout<<"!!!" << numMetaballs <<"\n";
  if(numMetaballs)
    GPGPUManager::GetSingleton().WriteBuffer(mpMetaballsBuffer, 0, numMetaballs, &metaballs[0]);
  //\ Eof metaballs buffer

  // Execute the kernel
  // Global work size (gws) of general kernel
  GPKernel *krnlBuildGenTerrain = mpTerrainProg->GetKernel("BuildTerrainPatch");

  // Set kernel arguments
  krnlBuildGenTerrain->SetArgument(0, mpTerrainVolumeWrite);
  krnlBuildGenTerrain->SetArgument(1, &mTerrainPatchSize);
  krnlBuildGenTerrain->SetArgument(2, &mGround);

  // Heightmap
  krnlBuildGenTerrain->SetArgument(3, &mHMEnable);
  krnlBuildGenTerrain->SetArgument(4, &mHMSampleY);
  krnlBuildGenTerrain->SetArgument(5, &mHMNumOctaves);
  krnlBuildGenTerrain->SetArgument(6, &mHmPersistance);
  krnlBuildGenTerrain->SetArgument(7, &mHmIntensity);

  // Height gradient
  krnlBuildGenTerrain->SetArgument(8, &mGradIntensity);
  krnlBuildGenTerrain->SetArgument(9, &mGradStart);

  // Hard ground
  krnlBuildGenTerrain->SetArgument(10, &mHardGroundIntensity);
  krnlBuildGenTerrain->SetArgument(11, &mHardGroundStart);

  // Noise layers buffer
  krnlBuildGenTerrain->SetArgument(12, mpNoiseLayersBuffer);
  krnlBuildGenTerrain->SetArgument(13, &numNoiseLayers);

  // Metaballs buffer
  krnlBuildGenTerrain->SetArgument(14, mpMetaballsBuffer);
  krnlBuildGenTerrain->SetArgument(15, &numMetaballs);

  // Pass the noise prefabs
  ZUInt32 numPrefabs = mNoisePrefabs.size();
  if(numPrefabs)
  {
    // TODO: parse the Noise prefabs 6 + index*2 + 1, ...+2
    // Pass all the noise prefabs
    NoisePrefab *nPrefab0 = mNoisePrefabs.find(0)->second;
    krnlBuildGenTerrain->SetArgument(16, nPrefab0->ptr);    // Prefab 0 - 3D image
    krnlBuildGenTerrain->SetArgument(17, &nPrefab0->size);  // Prefab 0 - size = 0

    NoisePrefab *nPrefab1 = mNoisePrefabs.find(1)->second;
    krnlBuildGenTerrain->SetArgument(18, nPrefab1->ptr);    // Prefab 0 - 3D image
    krnlBuildGenTerrain->SetArgument(19, &nPrefab1->size);  // Prefab 0 - size = 0

    NoisePrefab *nPrefab2 = mNoisePrefabs.find(2)->second;
    krnlBuildGenTerrain->SetArgument(20, nPrefab2->ptr);    // Prefab 0 - 3D image
    krnlBuildGenTerrain->SetArgument(21, &nPrefab2->size);  // Prefab 0 - size = 0
  }

  // Work group, etc
  ZUInt32 tWorkDimension     = 2;
  size_t  tLocalWorkSize[3]  = { 16, 16 };
  size_t  tGlobalWorkSize[3] = {VoxelHelper::GetSingleton().RoundUp(tLocalWorkSize[0], mTerrainPatchSize),
                                VoxelHelper::GetSingleton().RoundUp(tLocalWorkSize[1], mTerrainPatchSize) };

  // Execute the kernel
  GPGPUManager::GetSingleton().ExecuteKernel(krnlBuildGenTerrain, tWorkDimension, NULL, tGlobalWorkSize, tLocalWorkSize);

  // temp
  //std::vector<Math::Vector4> nn(4);
  //GPGPUManager::GetSingleton().ReadBuffer(mpNoiseLayersBuffer, 0, 1, &nn[0]);
  //\temp

  // Copy terrain grid as read only
  // This holds a 3D grid of the general terrain structure
  std::vector<float> terrainBufferVec(mTerrainPatchSize*mTerrainPatchSize*mTerrainPatchSize);
  GPGPUManager::GetSingleton().ReadImage(mpTerrainVolumeWrite, terrainGridOrigin, terrainGridRegion, &terrainBufferVec[0], true);

  // Copy to the terrain Read volume
  mpTerrainVolumeRead = GPGPUManager::GetSingleton().CreateImage(
                  "TerrainPatchVolume_Read", 
                  MA_READ_ONLY, MF_COPY_HOST_PTR,
                  MOT_IMAGE3D, ICO_R, ICDT_FLOAT,
                  mTerrainPatchSize, mTerrainPatchSize, mTerrainPatchSize, 
                  &terrainBufferVec[0]);

  // Release the Write volume
  GPGPUManager::GetSingleton().ReleaseMemObject("TerrainPatchVolume_Write");
  //mpTerrainVolumeWrite->Release();
  mpTerrainVolumeWrite = NULL;
}

void VoxelTerrainManager::InitializeBuffers(ZUInt32 posAOGLBuffer, ZUInt32 normGLBuffer, ZUInt32 texCoords01GLBuffer)
{
  // Initialize the shared GLBuffers
  mPosBuffer        = GPGPUManager::GetSingleton().CreateFromHardwareBuffer(
                        "PosAORenderBuffer", MA_WRITE_ONLY, MF_NONE, posAOGLBuffer);

  //mPosBuffer        = GPGPUManager::GetSingleton().CreateBuffer(
  //                      "PosAORenderBuffer", MA_WRITE_ONLY, MF_NONE, AT_FLOAT4, numVertices, NULL);

  mNormBuffer       = GPGPUManager::GetSingleton().CreateFromHardwareBuffer(
                        "NormRenderBuffer", MA_WRITE_ONLY, MF_NONE, normGLBuffer);
  //
  //mTexCoord01Buffer = GPGPUManager::GetSingleton().CreateFromHardwareBuffer(
  //                      "TexCoord01RenderBuffer",MA_WRITE_ONLY, MF_NONE, texCoords01GLBuffer);

  // Initialize the noise layers 
  mpNoiseLayersBuffer   = GPGPUManager::GetSingleton().CreateBuffer(
                            "NoiseLayers", MA_READ_ONLY, MF_ALLOC_HOST_PTR, AT_FLOAT4, 32, NULL);

  mpMetaballsBuffer     = GPGPUManager::GetSingleton().CreateBuffer(
                            "Metaballs", MA_READ_ONLY, MF_ALLOC_HOST_PTR, AT_FLOAT8, 32, NULL);
}

void VoxelTerrainManager::ShutdownBuffers()
{
  // Cleanup the buffers
  //if(mpNoiseLayersBuffer)
  //{
  //  mpNoiseLayersBuffer->Release();
  //  mpNoiseLayersBuffer = NULL;
  //}
  //
  //if(mNormBuffer)
  //{
  //  mNormBuffer->Release();
  //  mNormBuffer = NULL;
  //}
  //
  //if(mPosBuffer)
  //{
  //  mPosBuffer->Release();
  //  mPosBuffer = NULL;
  //}
}

void VoxelTerrainManager::ClearNoisePrefabs()
{
  // Parse the map and clean all
  NoisePrefabsMapPtr::iterator itNP;

  for(itNP=mNoisePrefabs.begin(); itNP != mNoisePrefabs.end(); itNP++)
  {
    // Get the prefab params
    NoisePrefab *prefabParams = (*itNP).second;

    if(prefabParams->ptr)
    {
      delete prefabParams;
      prefabParams = NULL;
    }
  }

  mNoisePrefabs.clear();
}

void VoxelTerrainManager::ClearTerrainVolume()
{
  // Read volume
  if(mpTerrainVolumeRead)
  {
    //mpTerrainVolumeRead->Release();
    mpTerrainVolumeRead = NULL;
  }

  // Write volume
  if(mpTerrainVolumeWrite)
  {
    //mpTerrainVolumeWrite->Release();
    mpTerrainVolumeWrite = NULL;
  }
}