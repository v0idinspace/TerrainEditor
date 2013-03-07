#include "FrameObserver.h"
#include <Input\Win32\SCTMouseWin32.h>
#include <Input\Win32\SCTKeyboardWin32.h>
#include <Render\Cg\ZephyrCgEffect.h>


// NOTE: this includes will be removed once everything is accessible 
// through the engine class or singleton get calls
#include <Render\Cg\ZephyrCgShader.h>


using namespace Zephyr;
using namespace Core;
using namespace GPGPU;
using namespace Geometry;
using namespace Math;
using namespace Render;
using namespace Scene;


#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))


vtmFrameObserver::vtmFrameObserver() //RenderWindow* mainWindow
: mpLightingFX(NULL),
  mpColorFX(NULL)
{
  // Initialize variables
  mNumVertices    = 5000000;            // Max: 5KK vertices
  mCLProgramsDir  = "CLPrograms\\";
  mShadersDir     = "Shaders\\";

  // Initialize -- CL before vbo
  InitGPGPU();

  // Voxel terrain manager
  VoxelTerrainManager *vtm = new VoxelTerrainManager;
  
  // load vs, fs and setup vbo
  LoadShaders();

  // initialize the posAo, Normal, texCoord01
  InitGLRenderBuffers();

  // Initialize the voxel terrain manager
  vtm->Initialize(mPosAoGLBufferID, mNormGLBufferID, mTexCoord01GLBufferID, mNumVertices, 5);

  // Setup the noise prefabs
  GUI::GetSingleton().GenerateNoisePrefabs();

  vtm->SetTerrainParameters(GUI::GetSingleton().mTPSize, 
                            GUI::GetSingleton().mGround, 0, 0, 0);
  vtm->GenerateTerrainPatch();

  // Initialize the camera
	mpCameraFree	= new OrbitCamera;
	mpCameraFree->SetProjectionMatrix(float(ZEPHYR_MATH_PI/4), float(800/600), 0.1f, 1000.0f);
	
  mpCameraFree->SetPosition(0, 0, -200.0f);
  mpCameraFree->SetTarget(Vector3(0, 0, 0));
	mpCameraFree->UseFrustum(false);
  mpCameraFree->SetType(CT_ORBIT);
  mCameraSpeed        = 1.0f;
  mCameraSensitivity  = 3.0f;

  // Enable the GUI
  GUI::GetSingleton().Enabe(true);

  mpLightingFX->SetParameterValue("gProjectionMtx", (float*)&mpCameraFree->GetProjectionMatrix());
  mpColorFX->SetParameterValue("gProjectionMtx", (float*)&mpCameraFree->GetProjectionMatrix());
}

void vtmFrameObserver::Initialize()
{
}

void vtmFrameObserver::Shutdown()
{
  // Shutdown VoxelTerrainManager
  VoxelTerrainManager *vtm = VoxelTerrainManager::GetSingletonPtr();
  delete vtm;
  vtm = NULL;

  // Cleanup
  delete mpCameraFree;
  mpCameraFree = NULL;

  // Shutdown the GPGPU module
  ShutdownGPGPU();
}

vtmFrameObserver::~vtmFrameObserver()
{
}

bool vtmFrameObserver::NotifyFrameStarted()
{
  // GL stuff
  glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
  glClearDepth(1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Enable Depth buffer
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  
  // Enable blending
  glEnable (GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  // Enable backface bulling
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  //\ Eof GL

  // Handle input
  HandleInput();

  // Update the scene
  Update();

  // Render the scene
  RenderScene(mWorldMtx);
  
  // Render wireframe
  if(GUI::GetSingleton().mbWireframe)
    RenderWireframe(mWorldMtx);

  return true;
}

void vtmFrameObserver::HandleInput()
{
  float dt = Zephyr::Core::Timer::GetSingleton().GetElapsedTime();

  Input::SCTInputManager::GetSingleton().Update();

  Input::SCTKeyboard	*keyboard	  = Input::SCTInputManager::GetSingleton().GetKeyboard();
	Input::SCTMouse		  *mouse		  = Input::SCTInputManager::GetSingleton().GetMouse();

  // Reset the camera
  if(keyboard->IsKeyPressed(Input::SCT_R))
  {
      mpCameraFree->SetPosition(0, 200.0f, 0);
	    mpCameraFree->SetTarget(Vector3(0, 0, 0));
      mpCameraFree->UseFrustum(false);
      mpCameraFree->SetType(CT_ORBIT);
      mpCameraFree->Reset();
      mCameraSpeed        = 1.0f;
      mCameraSensitivity  = 3.0f;
  }

  // Move the camera
  if(keyboard->GetKeyState(Input::SCT_W))
  {
    //mpCameraFree->OrbitRotate(0, dt * mCameraSensitivity);
    //mpCameraFree->Move(mCameraSpeed * dt);
  }
  if(keyboard->GetKeyState(Input::SCT_S))
  {
    //mpCameraFree->OrbitRotate(0, dt * mCameraSensitivity);
    //mpCameraFree->Move(-mCameraSpeed * dt);
  }
 

  if(keyboard->GetKeyState(Input::SCT_A))
    mpCameraFree->MovePerpendicular(-mCameraSpeed * dt, 0);
  if(keyboard->GetKeyState(Input::SCT_D))
    mpCameraFree->MovePerpendicular(mCameraSpeed * dt, 0);

  if(mouse->GetMouseState(Input::SCT_MOUSE_LEFT))
	{
    // Orbit camera
    mpCameraFree->OrbitRotate(
      mouse->GetRelativeX() * dt * mCameraSensitivity, 
      mouse->GetRelativeY() * dt * mCameraSensitivity);

    //mpCameraFree->OrbitRotateHorizontal(-mouse->GetRelativeX() * dt * mCameraSensitivity);
		//mpCameraFree->RotateYaw(mouse->GetRelativeX() * dt * mCameraSensitivity);
		//mpCameraFree->RotatePitch(mouse->GetRelativeY() * dt * mCameraSensitivity);
	}

	//// Camera Zoom in / out with mouse right
	if(mouse->GetMouseState(Input::SCT_MOUSE_RIGHT))
  {
		mpCameraFree->Move(-mouse->GetRelativeX() * dt * 2.0f);
  }

  //keyboard->IsKeyReleased(Input::SCT_C);
  keyboard->IsKeyReleased(Input::SCT_R);
}

void vtmFrameObserver::Update()
{
  // Handles scene update etc...

  // Update the camera
  mpCameraFree->Update();
  
  // Center the terrain patch to 000
  // Get terrain patch size
  static float patchSizeOfst = 32.0f;
  float curTPS = (float)VoxelTerrainManager::GetSingleton().GetTerrainPatchSize()/2.0f;

  if(curTPS != patchSizeOfst)
    patchSizeOfst = curTPS;

  Matrix4 trans;
  trans.SetTranslation(-patchSizeOfst, -patchSizeOfst, -patchSizeOfst);

  // Rotate the terrain patch around 000 
  if(GUI::GetSingleton().mbAutoRotate)
  {
    Matrix4 rot;
    static float angle = 0;
    float dt = Core::Timer::GetSingleton().GetElapsedTime();

    angle += 0.4f * dt;
    rot.SetRotationYawPitchRoll(angle, 0, 0);

    mWorldMtx = trans * rot;
  }
  else
  {
    mWorldMtx = trans;
  }
}

void vtmFrameObserver::RenderScene(Math::Matrix4 &localToWorldMtx)
{
  // Cgfx params
  Vector3 lightColor(1.0f, 1.0f, 1.0f),
          lightPosition(0, 0, 0);

  // Get light direction from the GUI
  lightPosition = GUI::GetSingleton().mLightDirection * Vector3(250.0f, -250.0f, 250.0f);

  mpLightingFX->SetParameterValue("gLightColor" ,     (float*)&lightColor);
  mpLightingFX->SetParameterValue("gLightPosition" ,  (float*)&lightPosition);
  mpLightingFX->SetParameterValue("gViewMtx",         (float*)&mpCameraFree->GetViewMatrix());
  mpLightingFX->SetParameterValue("gWorldMtx",        (float*)&localToWorldMtx);

  // Render using the cgfx technique
  Render::Technique *tech = mpLightingFX->GetTechnique("LightingNoAlpha");

  if(tech)
  {
    for(ZUInt32 i=0; i < tech->GetNumPasses(); i++)
    {
      Render::CgPass *pass = static_cast<Render::CgPass*>(tech->GetPass(i));
      cgSetPassState(pass->GetCgPass());

      // --- Bind buffer - Position ---
      void* pBufferData = 0;

      glBindBuffer(GL_ARRAY_BUFFER, mPosAoGLBufferID);
      pBufferData = VBO_BUFFER_OFFSET(0);

      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4), pBufferData);

      ShaderParameter *param = pass->GetInputLayout()->GetParameter(IES_POSITION);
      CGparameter cgInputParam      = static_cast<Render::CgShaderParameter*>(param)->GetCgParameter();

      // Bind to the Vertex shader (using the InputLayout)
      cgGLEnableClientState(cgInputParam);
      cgGLSetParameterPointer(cgInputParam, 4, GL_FLOAT, sizeof(Vector4), pBufferData);

      // --- Bind buffer - Normal ---
      glBindBuffer(GL_ARRAY_BUFFER, mNormGLBufferID);
      pBufferData = VBO_BUFFER_OFFSET(0);

      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vector4), pBufferData);

      param = pass->GetInputLayout()->GetParameter(IES_NORMAL);
      cgInputParam      = static_cast<Render::CgShaderParameter*>(param)->GetCgParameter();

      // Bind to the Vertex shader (using the InputLayout)
      cgGLEnableClientState(cgInputParam);
      cgGLSetParameterPointer(cgInputParam, 3, GL_FLOAT, sizeof(Vector4), pBufferData);

      // --- Render ---
      glDisable(GL_CULL_FACE);
      //glCullFace(GL_FRONT);

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      // Get GL error
      String glErrorString;
      GLenum glErrorCode = glGetError();
      glErrorString = (char*)gluErrorString(glErrorCode);
      //\ Eof get GL error

      glDrawArrays(GL_TRIANGLES, 0, mNumVertices);//GL_TRIANGLE_STRIP

      glDisableVertexAttribArray(0);
      //glDisableVertexAttribArray(3);

      // TODO: Dunno if i need this, probably yes
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      //\Eof render

      cgResetPassState(pass->GetCgPass());
    }
  }
}

void vtmFrameObserver::RenderWireframe(Math::Matrix4 &localToWorldMtx)
{
  // Cgfx
  mpColorFX->SetParameterValue("gViewMtx", (float*)&mpCameraFree->GetViewMatrix());
  mpColorFX->SetParameterValue("gWorldMtx", (float*)&localToWorldMtx);

  // Render using the cgfx technique
  Render::Technique *tech = mpColorFX->GetTechnique("WireframeAlpha");

  if(tech)
  {
    for(ZUInt32 i=0; i < tech->GetNumPasses(); i++)
    {
      Render::CgPass *pass = static_cast<Render::CgPass*>(tech->GetPass(i));
      cgSetPassState(pass->GetCgPass());

      // --- Bind buffer - Position ---
      void* pBufferData = 0;

      glBindBuffer(GL_ARRAY_BUFFER, mPosAoGLBufferID);
      pBufferData = VBO_BUFFER_OFFSET(0);

      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4), pBufferData);

      ShaderParameter *param = pass->GetInputLayout()->GetParameter(IES_POSITION);
      CGparameter cgInputParam      = static_cast<Render::CgShaderParameter*>(param)->GetCgParameter();

      // Bind to the Vertex shader (using the InputLayout)
      cgGLEnableClientState(cgInputParam);
      cgGLSetParameterPointer(cgInputParam, 4, GL_FLOAT, sizeof(Vector4), pBufferData);

      // --- Render ---     
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      
      // Get GL error
      String glErrorString;
      GLenum glErrorCode = glGetError();
      glErrorString = (char*)gluErrorString(glErrorCode);
      //\ Eof get GL error

      glDrawArrays(GL_TRIANGLES, 0, mNumVertices);//GL_TRIANGLE_STRIP

      glDisableVertexAttribArray(0);
      //glDisableVertexAttribArray(3);

      // TODO: Dunno if i need this, probably yes
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      //\Eof render

      cgResetPassState(pass->GetCgPass());
    }
  }
}

void vtmFrameObserver::InitGPGPU()
{
  // GPGPU Manager
  GPGPUManager *gpgpu = new GPGPUManager;

  // Init GPGPU manager
  GPGPUManager::GetSingleton().QueryPlatformsAndDevices();

  std::cout << "$> Displaying platform/device info...\n\n";

  // Print platform/device info
  for(ZUInt32 itPlat=0; itPlat<GPGPUManager::GetSingleton().GetNumberOfPlatforms(); itPlat++)
  {
    GPPlatform platform = GPGPUManager::GetSingleton().GetPlatform(itPlat);
    
    // Get & display formated platform info
    String platInfo = GPHelper::GetSingleton().GetFormatedPlatformInfo(platform, itPlat);
    std::cout << platInfo << "\n";

    // Iterate throught the current platforms devices & display info
    for(ZUInt32 itDev=0; itDev<GPGPUManager::GetSingleton().GetNumberOfDevices(); itDev++)
    {
      String devInfo = GPHelper::GetSingleton().GetFormatedDeviceInfo(GPGPUManager::GetSingleton().GetDevice(itDev), itDev);
      std::cout << devInfo << "\n";
    }

    std::cout << "-----------------------------------------------\n";
  }

  GPGPUManager::GetSingleton().Initialize(0, 0);
}

void vtmFrameObserver::ShutdownGPGPU()
{
  GPGPUManager *gpgpu = GPGPUManager::GetSingletonPtr();
  delete gpgpu;
  gpgpu = NULL;
}

void vtmFrameObserver::InitGLRenderBuffers()
{
  mPosAoGLBufferID;
  //mNormGLBufferID;
  //mTexCoord01GLBufferID;


  // Shared Vbo
  Vector4 *testData = new Vector4[mNumVertices];
  
  //testData[0] = Vector4(0, 0, 0, 1);
  testData[1] = Vector4(0, 0, 1, 1);
  testData[2] = Vector4(0, 0, 1, 1);
  testData[3] = Vector4(0, 0, 1, 1);
  testData[4] = Vector4(0, 0, 1, 1);
  testData[5] = Vector4(0, 0, 1, 1);
  testData[6] = Vector4(0, 0, 1, 1);
  //testData[3] = Vector4(1, 0, 0, 1);
  //testData[4] = Vector4(0, 1, 0, 1);
  //testData[5] = Vector4(2, 0, 0, 1);
  
  glGenBuffers(1, &mPosAoGLBufferID);
  glBindBuffer(GL_ARRAY_BUFFER, mPosAoGLBufferID);
  glBufferData(GL_ARRAY_BUFFER, mNumVertices*sizeof(Vector4), testData, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Normal Vbo
  glGenBuffers(1, &mNormGLBufferID);
  glBindBuffer(GL_ARRAY_BUFFER, mNormGLBufferID);
  glBufferData(GL_ARRAY_BUFFER, mNumVertices*sizeof(Vector4), testData, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  delete []testData;
}

void vtmFrameObserver::LoadShaders()
{
  // CGFX 
  String cgfxData = Core::FileSystem::GetSingleton().GetFileData("Shaders\\Lighting.cgfx");
  Render::ShaderManager::GetSingleton().CreateEffect("LightingFX", cgfxData);

  mpLightingFX = Render::ShaderManager::GetSingleton().GetEffect("LightingFX");

  cgfxData = Core::FileSystem::GetSingleton().GetFileData("Shaders\\Color.cgfx");
  Render::ShaderManager::GetSingleton().CreateEffect("ColorFX", cgfxData);

  mpColorFX = Render::ShaderManager::GetSingleton().GetEffect("ColorFX");
}