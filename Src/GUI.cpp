#include <GUI.h>
#include <Statistics.h>
#include <OrbitCamera.h>


using namespace Zephyr;


// GUI Callbacks
void TW_CALL cbCreateNoisePrefab(void *clientData);
void TW_CALL cbUpdateNoisePrefab(void *clientData);
void TW_CALL cbDeleteNoisePrefab(void *clientData);
void TW_CALL cbAddLayerToTerrain(void *clientData);
void TW_CALL cbAddMetaballToTerrain(void *clientData);
void TW_CALL cbRemoveLayerFromTerrain(void *clientData);
void TW_CALL cbRemoveMetaballFromTerrain(void *clientData);
void TW_CALL cbUpdateTerrain(void *clientData);
void TW_CALL cbSaveFile(void *clientData);


GUI::GUI(ZUInt32 windowWidth, ZUInt32 windowHeight)
: mbIsVTMInitialized(false),
  mpTwTerrainDlg (NULL),
  mpTwNoiseDlg(NULL),
  mpTwStatsDlg(NULL)
{
  TwInit(TW_OPENGL, NULL);
  TwWindowSize(windowWidth, windowHeight);

  // Create the GUI
  mpTwTerrainDlg = TwNewBar("TerrainParams");
  mpTwNoiseDlg   = TwNewBar("NoisePrefabs");
  mpTwStatsDlg   = TwNewBar("Stats");

  // Left bars
  TwDefine("  TerrainParams label='Terrain' \
              color='32 64 64' alpha=128 \
              position='5 5' \
              size='225 600' \
              movable=false \
              resizable=false \
              alwaysbottom=true ");
  
  // Right bars
  TwDefine("  Stats label='Stats' \
              color='0 0 0' alpha=0 \
              position='794 5' \
              size='225 150' \
              movable=false \
              resizable=false ");
  
  TwDefine("  NoisePrefabs label='Noise prefabs' \
              color='64 32 64' alpha=64 \
              position='794 300' \
              size='225 335' \
              movable=false \
              resizable=false \
              alwaysbottom=true ");
  
  
  // Keeps track of the current terrain layer index
  mLayerIndex           = 0;
  mMetaballIndex        = 0;

  // Initialize terrain patch params
  mTPSize               = 64;
  mIsoValue             = 0.25f;
  mGround               = 8;

  mGenHeightmap         = false;
  mHmSampleY            = 0;
  mHmNumOctaves         = 1;
  mHmPersistance        = 1.0f;
  mHmIntensity          = 1.0f;

  mGradIntensity        = 0;
  mGradStart            = 0;

  mHardGroundStart      = 0;
  mHardGroundIntensity  = 0;

  // Stats / Auto-Rotate
  mFPS                  = 0;
  mMsPerFrame           = 0;
  mCPUUsage             = 0;
  mNumTriangles         = 0;
  mbAutoRotate          = false;
  mbWireframe           = false;

  mLightDirection       = Math::Vector3(0, -1.0f, 0);
}

void GUI::Initialize()
{
  // Define enum type
  mNoiseLayerType        = TwDefineEnum("NoiseLayerType", NULL, 0);

  // Stats
  TwAddVarRO(mpTwStatsDlg,  "fps", TW_TYPE_FLOAT, &mFPS, " label='FPS' ");
  TwAddVarRO(mpTwStatsDlg,  "msPerFrame", TW_TYPE_FLOAT, &mMsPerFrame, " label='Ms per frame' ");
  TwAddVarRO(mpTwStatsDlg,  "cpuUsage", TW_TYPE_UINT32, &mCPUUsage, " label='CPU usage' ");
  TwAddVarRO(mpTwStatsDlg,  "numTriangles", TW_TYPE_UINT32, &mNumTriangles, " label='Triangles' ");
  TwAddVarRW(mpTwStatsDlg,  "autoRotate", TW_TYPE_BOOL8, &mbAutoRotate, " label='Auto rotate: ' true=Yes false=No ");
  TwAddVarRW(mpTwStatsDlg,  "wireframe", TW_TYPE_BOOL8, &mbWireframe, " label='Wireframe: ' true=Yes false=No ");
  TwAddVarRW(mpTwStatsDlg,  "lightDirection", TW_TYPE_DIR3F, &mLightDirection, " label='Light dir: '");

  // Terrain related parameters
  TwAddButton(mpTwTerrainDlg, "TGP", NULL, NULL, " label='General params' ");
  TwAddVarRW(mpTwTerrainDlg,  "TGPsize", TW_TYPE_UINT32, &mTPSize, " label='Size' step=1 max=128");

  // IsoValue
  std::ostringstream guiElParams;
  guiElParams.str("");
  guiElParams << " label='IsoValue' step=0.01 min=0 max=1" << mIsoValue;
  TwAddVarRW(mpTwTerrainDlg,  "TGPIsovalue", TW_TYPE_FLOAT, &mIsoValue, guiElParams.str().c_str());

  // Ground
  guiElParams.str("");
  guiElParams << " label='Ground' step=1 max=" << mTPSize;
  TwAddVarRW(mpTwTerrainDlg,  "TGPground", TW_TYPE_UINT32, &mGround, guiElParams.str().c_str());

  // Heightmap
  //TwAddButton(mpTwTerrainDlg, "HM", NULL, NULL, " label='Heightmap' group='heightmap' ");
  //TwAddVarRW(mpTwTerrainDlg,  "HMGenerate", TW_TYPE_BOOLCPP, &mGenHeightmap, " label='Generate' group='heightmap' ");
  //TwAddVarRW(mpTwTerrainDlg,  "HMSampleY", TW_TYPE_UINT32, &mHmSampleY, " label='Sample Y' min=0 max=32 group='heightmap' ");
  //TwAddVarRW(mpTwTerrainDlg,  "HMNumOctaves", TW_TYPE_UINT32, &mHmNumOctaves, " label='Num Octaves' min=0 max=16 step=1 group='heightmap' ");
  //TwAddVarRW(mpTwTerrainDlg,  "HMPersistance", TW_TYPE_FLOAT, &mHmPersistance, " label='Persistance' min=0 max=1.0 step=0.01 group='heightmap' ");
  //TwAddVarRW(mpTwTerrainDlg,  "HMIntensity", TW_TYPE_FLOAT, &mHmIntensity, " label='Intensity' min=0 max=1.0 step=0.01 group='heightmap' ");
  
  // Layers
  TwAddButton(mpTwTerrainDlg, "NLinit", NULL, NULL, " label='No layers' group='NoiseLayers' ");

  // Metaballs
  TwAddButton(mpTwTerrainDlg, "Minit", NULL, NULL, " label='No metaballs' group='Metaballs' ");

  // Gradient
  //TwAddButton(mpTwTerrainDlg, "HG1", NULL, NULL, " label='Height gradient' group='Height gradient' ");
  TwAddVarRW(mpTwTerrainDlg,  "HG1start", TW_TYPE_UINT32, &mGradStart, " label='Start' step=1 group='HeightGradient' ");
  TwAddVarRW(mpTwTerrainDlg,  "HG1intensity", TW_TYPE_FLOAT, &mGradIntensity, " label='Intensity' step=0.01 group='HeightGradient' min=0");

  // Hard ground
  TwAddVarRW(mpTwTerrainDlg,  "HaG1start", TW_TYPE_UINT32, &mHardGroundStart, " label='Start' step=1 min=0 group='HardGround' ");
  TwAddVarRW(mpTwTerrainDlg,  "HaG1intensity", TW_TYPE_FLOAT, &mHardGroundIntensity, " label='Intensity' step=0.01 min=0 group='HardGround' ");

  TwAddSeparator(mpTwTerrainDlg, "BtnSep", "");
  TwAddButton(mpTwTerrainDlg, "AddLayer", cbAddLayerToTerrain, NULL, " label='Add noise layer'");
  TwAddButton(mpTwTerrainDlg, "AddMetaball", cbAddMetaballToTerrain, NULL, " label='Add metaball'");
  TwAddButton(mpTwTerrainDlg, "Update", cbUpdateTerrain, NULL, " label='Update Terrain'");
  TwAddButton(mpTwTerrainDlg, "Savefile", cbSaveFile, NULL, " label='Save file'");
}

void GUI::Update()
{
  // Update the statistics
  Statistics::GetSingleton().GetFrameStats(mFPS, mMsPerFrame);
  Statistics::GetSingleton().GetCpuPercentage(mCPUUsage);
  mNumTriangles = VoxelTerrainManager::GetSingleton().GetCurrentNumVertices() / 3;

  TwDraw();
}

void GUI::Shutdown()
{
  // Clear noise prefabs map
  ClearNoisePrefabs();
  
  // Clear noise layers map
  ClearNoiseLayers();

  // Shutdown Tw
  TwTerminate();
}

void GUI::GenerateNoisePrefabs()
{
  for(ZUInt32 i=0; i<3; i++)
  {
    // Noise prefab
    NoisePrefab *nPrefab = new NoisePrefab;

    // Configure
    String name;
    std::ostringstream  twName, twParam;

    nPrefab->index      = i;
    nPrefab->ptr        = NULL;  // we don't care about that 
    nPrefab->size       = 32;
    nPrefab->frequency  = 1.0f;

    mNoisePrefabs.insert(NoisePrefabMapPair(i, nPrefab));

    // Title
    twName.str("");
    twName << "NS" << i;
    name = twName.str().c_str();

    twParam.str("");
    twParam << " label='Noise prefab " << i << "' ";
    TwAddButton(mpTwNoiseDlg, name.c_str(), NULL, NULL, twParam.str().c_str());

    // Size param
    twName.str("");
    twName << name << "size";
    TwAddVarRW(mpTwNoiseDlg, twName.str().c_str(), TW_TYPE_UINT32, &nPrefab->size, "step=1");

    // Frequency param
    twName.str("");
    twName << name << "freq";
    TwAddVarRW(mpTwNoiseDlg, twName.str().c_str(), TW_TYPE_FLOAT, &nPrefab->frequency, "step=0.1 min=0");

    // Update button
    twName.str("");
    twName << name << "update";
    TwAddButton(mpTwNoiseDlg, twName.str().c_str(), cbUpdateNoisePrefab, &nPrefab->index, " label='Update' ");

    // Separator
    twName.str("");
    twName << name << "sep";
    TwAddSeparator(mpTwNoiseDlg, twName.str().c_str(), "");

    // Notify the terrain manager
    VoxelTerrainManager::GetSingleton().AddNoisePrefab(i, nPrefab->size, nPrefab->frequency);
  }
}

void GUI::ClearNoisePrefabs()
{
  // Parse the map and clean all
  NoisePrefabsMapPtr::iterator itNP;

  for(itNP = mNoisePrefabs.begin(); itNP != mNoisePrefabs.end(); itNP++)
  {
    // Get the prefab params
    NoisePrefab *prefabParams = (*itNP).second;

    if(prefabParams)
    {
      //prefabParams->ptr->Release();    // Release the CL buffer
      delete prefabParams;
      prefabParams = NULL;
    }
  }

  mNoisePrefabs.clear();
}

void GUI::ClearNoiseLayers()
{
  // Parse the map and clean all
  NoiseLayersMapPtr::iterator itNL;

  for(itNL = mNoiseLayers.begin(); itNL != mNoiseLayers.end(); itNL++)
  {
    // Get the prefab params
    NoiseLayerParams *nLayer = (*itNL).second;

    if(nLayer)
    {
      //prefabParams->ptr->Release();    // Release the CL buffer
      delete nLayer;
      nLayer = NULL;
    }
  }

  mNoiseLayers.clear();
}


void GUI::Enabe(bool mode)
{ 
  mbIsVTMInitialized = mode; 

  // Debug camera vectors
  //TwAddVarRO(mpTwStatsDlg,  "LookAt", TW_TYPE_DIR3F, &OrbitCamera::GetSingleton().mLookAt, " label='LookAt: ' ");
  //TwAddVarRO(mpTwStatsDlg,  "Up", TW_TYPE_DIR3F, &OrbitCamera::GetSingleton().mUp, " label='Up: ' ");
  //TwAddVarRO(mpTwStatsDlg,  "Right", TW_TYPE_DIR3F, &OrbitCamera::GetSingleton().mRight, " label='Right: ' ");
}

int GUI::HandleWindowEvents(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  return TwEventWin(hwnd, message, wParam, lParam);
}

// Callbacks
void TW_CALL cbSaveFile(void *clientData)
{
  VoxelTerrainManager *vtm = VoxelTerrainManager::GetSingletonPtr();

  vtm->Export("TerrainPatchExport0.tpe");
}

void TW_CALL cbCreateNoisePrefab(void *clientData)
{
}

void TW_CALL cbUpdateNoisePrefab(void *clientData)
{
  GUI *gui = GUI::GetSingletonPtr();
  VoxelTerrainManager *vtm = VoxelTerrainManager::GetSingletonPtr();

  // Update the noise prefab
  if(vtm && gui)
  {
    ZUInt32 index = *(ZUInt32*)clientData;

    // Find the noise in the map
    NoisePrefabsMapPtr::iterator itNP = gui->mNoisePrefabs.find(index);

    if(itNP != gui->mNoisePrefabs.end())
    {
      NoisePrefab *prefabParams = (*itNP).second;

      // Print noise params
      //std::cout << ">$ ------------------------\n";
      //std::cout << ">$ Update Noise prefab: " << prefabParams->index << "\n";
      //std::cout << ">$ Size:\t\t"             << prefabParams->size << "\n";
      //std::cout << ">$ Frequency:\t"          << prefabParams->frequency << "\n";
      //std::cout << ">$ ------------------------\n";

      // Set the new parameters
      VoxelTerrainManager::GetSingleton().SetNoisePrefabParams(
        prefabParams->index, prefabParams->size, prefabParams->frequency);

      // Now update the terrain
      vtm->GenerateTerrainPatch();
    }
  }
}

void TW_CALL cbDeleteNoisePrefab(void *clientData)
{
}

void TW_CALL cbAddLayerToTerrain(void *clientData)
{
  GUI *gui = GUI::GetSingletonPtr();
  VoxelTerrainManager *vtm = VoxelTerrainManager::GetSingletonPtr();

  // Adds noise sample to terrain
  if(vtm && gui)
  {
    TwBar* terrainDlg = gui->mpTwTerrainDlg;
    ZUInt32 index = gui->mLayerIndex;

    // Initialize Layer values
    std::ostringstream name;
    name << "NS" << index;
    NoiseLayerParams *nLayer = new NoiseLayerParams;

    nLayer->name        = name.str().c_str();
    nLayer->enabled     = false;
    nLayer->type        = NLT_ADD;        // ADD
    nLayer->prefabIndex = 0;
    nLayer->scale       = 1.0f;     // relative to terrain size (1 = terrain size)
    nLayer->intensity   = 0.1f;

    // Insert to map
    gui->mNoiseLayers.insert(std::pair<String, NoiseLayerParams*>(nLayer->name, nLayer));

    // Add the GUI elements
    std::ostringstream guiElName, guiElParams;

    // Label
    guiElName.str("");
    guiElName << nLayer->name;
    guiElParams.str("");
    guiElParams << " label='Noise layer " << gui->mLayerIndex << "' group='NoiseLayers' ";
    TwAddButton(terrainDlg, guiElName.str().c_str(), NULL, NULL, guiElParams.str().c_str());

    // Enabled
    guiElName.str("");
    guiElName << nLayer->name << "enabled";
    guiElParams.str("");
    guiElParams << " label='Enabled " << gui->mLayerIndex << "' group='NoiseLayers' ";
    TwAddVarRW(terrainDlg, guiElName.str().c_str(), TW_TYPE_BOOLCPP, &nLayer->enabled, guiElParams.str().c_str());

    // Prefab index
    guiElName.str("");
    guiElName << nLayer->name << "prefab";
    guiElParams.str("");
    guiElParams << " label='Prefab' group='NoiseLayers' max=" << gui->mNoisePrefabs.size() - 1;  // todo: use enum
    TwAddVarRW(terrainDlg, guiElName.str().c_str(), TW_TYPE_UINT32, &nLayer->prefabIndex, guiElParams.str().c_str());

    // Noise type
    guiElName.str("");
    guiElName << nLayer->name << "type";
    guiElParams.str("");
    guiElParams << " label='Type' enum='0 {SUB}, 1 {ADD}' group='NoiseLayers' ";  // todo: use enum
    TwAddVarRW(terrainDlg, guiElName.str().c_str(), gui->mNoiseLayerType, &nLayer->type, guiElParams.str().c_str());

    // Noise scale
    guiElName.str("");
    guiElName << nLayer->name << "scale";
    guiElParams.str("");
    guiElParams << " label='Scale' step=0.01 group='NoiseLayers' min=0 ";
    TwAddVarRW(terrainDlg, guiElName.str().c_str(), TW_TYPE_FLOAT, &nLayer->scale, guiElParams.str().c_str());

    // Noise intensity
    guiElName.str("");
    guiElName << nLayer->name << "intensity";
    guiElParams.str("");
    guiElParams << " label='Intensity' step=0.01 group='NoiseLayers' min=0 ";
    TwAddVarRW(terrainDlg, guiElName.str().c_str(), TW_TYPE_FLOAT, &nLayer->intensity, guiElParams.str().c_str());

    // Remove button
    guiElName.str("");
    guiElName << nLayer->name << "remove";
    guiElParams.str("");
    guiElParams << " label='Remove' group='NoiseLayers' ";
    TwAddButton(terrainDlg, guiElName.str().c_str(), cbRemoveLayerFromTerrain, &nLayer->name, guiElParams.str().c_str());

    // Remove the init 'No layers' in NoiseLayers group (if this is the first element to be added)
    if(index == 0)
      TwRemoveVar(terrainDlg, "NLinit");

    // Notify the VTM
    vtm->AddNoiseLayer(nLayer->name, nLayer->enabled, nLayer->type, nLayer->prefabIndex, nLayer->scale, nLayer->intensity);

    // Increment the current layer index
    gui->mLayerIndex++;
  }
}

void TW_CALL cbRemoveLayerFromTerrain(void *clientData)
{
  String name = *(String*)clientData;

  GUI *gui = GUI::GetSingletonPtr();
  VoxelTerrainManager *vtm = VoxelTerrainManager::GetSingletonPtr();

  // Remove a noise layer from the terrain
  if(vtm && gui)
  {
    // Find the layer in the map
    std::map<String, NoiseLayerParams*>::iterator itL;
    itL = gui->mNoiseLayers.find(name);
    
    TwBar* terrainDlg = gui->mpTwTerrainDlg;

    if(itL != gui->mNoiseLayers.end())
    {
      NoiseLayerParams *nLayer = (*itL).second;
      
      // Remove from GUI
      // Add a 'No layers' flag to keep the position of the group in the bar as is
      if(gui->mLayerIndex == 0)
        TwAddButton(terrainDlg, "NLinit", NULL, NULL, " label='No layers' group='NoiseLayers' ");

      std::ostringstream twName;

      nLayer->name;
      TwRemoveVar(terrainDlg, nLayer->name.c_str());

      twName.str("");
      twName << nLayer->name << "enabled";
      TwRemoveVar(terrainDlg, twName.str().c_str());

      twName.str("");
      twName << nLayer->name << "type";
      TwRemoveVar(terrainDlg, twName.str().c_str());

      twName.str("");
      twName << nLayer->name << "prefab";
      TwRemoveVar(terrainDlg, twName.str().c_str());

      twName.str("");
      twName << nLayer->name << "scale";
      TwRemoveVar(terrainDlg, twName.str().c_str());

      twName.str("");
      twName << nLayer->name << "intensity";
      TwRemoveVar(terrainDlg, twName.str().c_str());

      twName.str("");
      twName << nLayer->name << "remove";
      TwRemoveVar(terrainDlg, twName.str().c_str());

      // Notify VTM
      vtm->RemoveNoiseLayer(nLayer->name);

      // Delete
      delete nLayer;

      // Remove from map
      gui->mNoiseLayers.erase(name);
      gui->mLayerIndex--;

      std::cout<<"$> Removed " << name << " layer from terrain\n";
    }
  }
}

void TW_CALL cbAddMetaballToTerrain(void *clientData)
{
  GUI *gui = GUI::GetSingletonPtr();
  VoxelTerrainManager *vtm = VoxelTerrainManager::GetSingletonPtr();

  // Adds noise sample to terrain
  if(vtm && gui)
  {
    TwBar* terrainDlg = gui->mpTwTerrainDlg;
    ZUInt32 index = gui->mLayerIndex;

    // Initialize Layer values
    std::ostringstream name;
    name << "M" << gui->mMetaballIndex;

    Metaball *metaball = new Metaball;

    metaball->name = name.str();
    metaball->position = Math::Vector3(0, 0, 0);
    metaball->rad = 0.5f;
    metaball->i   = 1.0f;

    // Insert to map
    gui->mMetaballs.insert(MetaballsMapPtrPair(name.str(), metaball));

    // Add the GUI elements
    std::ostringstream guiElName, guiElParams;

    // Label
    guiElName.str("");
    guiElName << name.str();
    guiElParams.str("");
    guiElParams << " label='Metaball " << gui->mMetaballIndex << "' group='Metaballs' ";
    TwAddButton(terrainDlg, guiElName.str().c_str(), NULL, NULL, guiElParams.str().c_str());

    // Enabled
    guiElName.str("");
    guiElName << name.str() << "enabled";
    guiElParams.str("");
    guiElParams << " label='Enabled " << gui->mMetaballIndex << "' group='Metaballs' ";
    TwAddVarRW(terrainDlg, guiElName.str().c_str(), TW_TYPE_BOOLCPP, &metaball->enabled, guiElParams.str().c_str());

    // Metaball position
    guiElName.str("");
    guiElName << name.str() << "positionx";
    guiElParams.str("");
    guiElParams << " label='x' group='Metaballs' step=1.0 "; 
    TwAddVarRW(terrainDlg, guiElName.str().c_str(), TW_TYPE_FLOAT, &metaball->position.x, guiElParams.str().c_str());

    guiElName.str("");
    guiElName << name.str() << "positiony";
    guiElParams.str("");
    guiElParams << " label='y' group='Metaballs' step=1.0 "; 
    TwAddVarRW(terrainDlg, guiElName.str().c_str(), TW_TYPE_FLOAT, &metaball->position.y, guiElParams.str().c_str());

    guiElName.str("");
    guiElName << name.str() << "positionz";
    guiElParams.str("");
    guiElParams << " label='z' group='Metaballs' step=1.0 "; 
    TwAddVarRW(terrainDlg, guiElName.str().c_str(), TW_TYPE_FLOAT, &metaball->position.z, guiElParams.str().c_str());

    // Metaball radious
    guiElName.str("");
    guiElName << name.str() << "radius";
    guiElParams.str("");
    guiElParams << " label='Radius' step=0.01 min=0 group='Metaballs' ";
    TwAddVarRW(terrainDlg, guiElName.str().c_str(), TW_TYPE_FLOAT, &metaball->rad, guiElParams.str().c_str());

    // Metaball intensity
    guiElName.str("");
    guiElName << name.str() << "intensity";
    guiElParams.str("");
    guiElParams << " label='Intensity' step=0.01 min=0 group='Metaballs'";
    TwAddVarRW(terrainDlg, guiElName.str().c_str(), TW_TYPE_FLOAT, &metaball->i, guiElParams.str().c_str());

    // Remove button
    guiElName.str("");
    guiElName << name.str() << "remove";
    guiElParams.str("");
    guiElParams << " label='Remove' group='Metaballs' ";
    TwAddButton(terrainDlg, guiElName.str().c_str(), cbRemoveMetaballFromTerrain, &metaball->name, guiElParams.str().c_str());

    // Remove the init 'No layers' in NoiseLayers group (if this is the first element to be added)
    if(gui->mMetaballIndex == 0)
      TwRemoveVar(terrainDlg, "Minit");

    // Notify the VTM
    vtm->AddMetaball(metaball->name, metaball->enabled, metaball->position, metaball->rad, metaball->i);

    // Increment the current layer index
    gui->mMetaballIndex++;
  }
}

void TW_CALL cbRemoveMetaballFromTerrain(void *clientData)
{
  String name = *(String*)clientData;

  GUI *gui = GUI::GetSingletonPtr();
  VoxelTerrainManager *vtm = VoxelTerrainManager::GetSingletonPtr();

  // Remove a noise layer from the terrain
  if(vtm && gui)
  {
    // Find the layer in the map
    MetaballsMapPtr::iterator itL;
    itL = gui->mMetaballs.find(name);
    
    TwBar* terrainDlg = gui->mpTwTerrainDlg;

    if(itL != gui->mMetaballs.end())
    {
      Metaball *metaball = (*itL).second;
      
      // Remove from GUI
      // Add a 'No layers' flag to keep the position of the group in the bar as is
      if(gui->mLayerIndex == 0)
        TwAddButton(terrainDlg, "Minit", NULL, NULL, " label='No metaballs' group='Metaballs' ");

      std::ostringstream twName;

      twName.str("");
      twName << name;
      TwRemoveVar(terrainDlg, twName.str().c_str());

      twName.str("");
      twName << name  << "enabled";
      TwRemoveVar(terrainDlg, twName.str().c_str());

      twName.str("");
      twName << name << "positionx";
      TwRemoveVar(terrainDlg, twName.str().c_str());

      twName.str("");
      twName << name << "positiony";
      TwRemoveVar(terrainDlg, twName.str().c_str());

      twName.str("");
      twName << name << "positionz";
      TwRemoveVar(terrainDlg, twName.str().c_str());


      twName.str("");
      twName << name << "radius";
      TwRemoveVar(terrainDlg, twName.str().c_str());

      twName.str("");
      twName << name << "intensity";
      TwRemoveVar(terrainDlg, twName.str().c_str());

      twName.str("");
      twName << name << "remove";
      TwRemoveVar(terrainDlg, twName.str().c_str());

      // Notify VTM
      vtm->RemoveMetaball(metaball->name);

      // Delete
      delete metaball;

      // Remove from map
      gui->mMetaballs.erase(name);
      gui->mMetaballIndex--;

      std::cout<<"$> Removed metaball " << name << " from terrain\n";
    }
  }
}

void TW_CALL cbUpdateTerrain(void *clientData)
{
  GUI *gui = GUI::GetSingletonPtr();
  VoxelTerrainManager *vtm = VoxelTerrainManager::GetSingletonPtr();

  // Update the terrain
  if(vtm && gui)
  {
    //std::cout<< "$> -----------------------------\n";
    //std::cout<< "$> Size:\t\t"               << gui->mTPSize << "\n";
    //std::cout<< "$> Ground:\t\t"             << gui->mGround << "\n";

    // Generate the terrain patch
    // Update the general terrain parameters
    VoxelTerrainManager::GetSingleton().SetTerrainParameters(
      gui->mTPSize, gui->mIsoValue, gui->mGround, gui->mGradIntensity, gui->mGradStart);

    //VoxelTerrainManager::GetSingleton().SertHeightmapParameters(gui->mGenHeightmap, 
    //                                                            gui->mHmSampleY, 
    //                                                            gui->mHmNumOctaves, 
    //                                                            gui->mHmPersistance,
    //                                                            gui->mHmIntensity);

    VoxelTerrainManager::GetSingleton().SetHardGround(gui->mHardGroundStart, gui->mHardGroundIntensity);

    // Update the Noise layer params
    NoiseLayersMapPtr::iterator itNL;
    for(itNL = gui->mNoiseLayers.begin(); itNL != gui->mNoiseLayers.end(); itNL++)
    {
      NoiseLayerParams *nl = (*itNL).second;
      vtm->SetNoiseLayerParams(nl->name, nl->type, nl->enabled, nl->prefabIndex, nl->scale, nl->intensity);

      //std::cout<< "$> Noise Layer:\t\t"   << nl->name         << "\n";
      //std::cout<< "$> Enabled:\t\t"       << nl->enabled      << "\n";
      //std::cout<< "$> Type:\t\t"          << nl->type         << "\n";
      //std::cout<< "$> Prefab Index:\t"    << nl->prefabIndex  << "\n";
      //std::cout<< "$> Scale:\t\t"         << nl->scale        << "\n";
      //std::cout<< "$> Intensity\t "       << nl->intensity    << "\n";
    }

    MetaballsMapPtr::iterator itM;
    for(itM = gui->mMetaballs.begin(); itM != gui->mMetaballs.end(); itM++)
    {
     Metaball *mb = (*itM).second;
     vtm->SetMetaballParams(mb->name, mb->enabled, mb->position, mb->rad, mb->i);
     
     std::cout<< "$> Metaball:\t\t"  << mb->name         << "\n";
     std::cout<< "$> Enabled:\t\t"   << mb->enabled      << "\n";
     std::cout<< "$> Position:\t\t"  << mb->position.x << ", " << mb->position.y << ", " << mb->position.z << "\n";
     std::cout<< "$> Radius:\t"      << mb->rad  << "\n";
     std::cout<< "$> Intensity\t "   << mb->i    << "\n";
    }
    
    //std::cout<< "$> Gradient start:\t"    << gui->mGradIntensity  << "\n";
    //std::cout<< "$> Gradient intensity: " << gui->mGradStart      << "\n";
    //std::cout<< "$> -----------------------------\n";

    vtm->GenerateTerrainPatch();
  }
}