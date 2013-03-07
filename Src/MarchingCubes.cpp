#include <MCDefines.h>
#include <MCTables.h>
#include <MarchingCubes.h>
#include <VoxelHelper.h>
#include <Math\ZephyrMath.h>


using namespace Zephyr;
using namespace GPGPU;


MarchingCubes::MarchingCubes()
: mNumVoxels(0),
  mpProgram(NULL),
  mpNumVertsToGenerate(NULL),
  mTotalNumVertices(NULL),
  mpPosBufferOffset(NULL),
  mpActiveVoxels(NULL),
  mpEdgeTable(NULL),
  mpTriTable(NULL),
  mpPositionVBO(NULL),
  mpNormalVBO(NULL),
  mpTexCoord01VBO(NULL),
  mMaxNumVertices(0),
  mNumVertices(0)
{
  // Load the program
  String programData = Core::FileSystem::GetSingleton().GetFileData("CLPrograms\\mc_simple.cl", Core::FF_TEXT);
  mpProgram = GPGPUManager::GetSingleton().LoadProgram("MarchingCubes", programData);
}

MarchingCubes::~MarchingCubes()
{
  Shutdown();
}

void MarchingCubes::Initialize(ZUInt32 volumeSize)
{
  // Initialize variables
  mVolumeSize = volumeSize;
  mNumVoxels  = mVolumeSize * mVolumeSize * mVolumeSize;

  // Initialize the buffers
  InitializeBuffers();
}

void MarchingCubes::Shutdown()
{
  // Shutdown buffers
  ShutdownBuffers();
}

void MarchingCubes::InitializeBuffers()
{
  // 2nd Marching cubes
  mpNumVertsToGenerate  = GPGPUManager::GetSingleton().CreateBuffer(
                          "NumVertsToGenerate", MA_READ_WRITE, MF_NONE, AT_UINT, mNumVoxels, NULL);

  mpPosBufferOffset     = GPGPUManager::GetSingleton().CreateBuffer(
                          "PosBufferOffset", MA_READ_WRITE, MF_NONE, AT_UINT, mNumVoxels, NULL);

  mpActiveVoxels        = GPGPUManager::GetSingleton().CreateBuffer(
                          "ActiveVoxels", MA_READ_WRITE, MF_NONE, AT_UINT, mNumVoxels, NULL);

  mpEdgeTable           = GPGPUManager::GetSingleton().CreateBuffer(
                          "EdgeTable", MA_READ_WRITE, MF_COPY_HOST_PTR, AT_UINT, 256, gCaseTable);

  mpTriTable            = GPGPUManager::GetSingleton().CreateBuffer(
                          "TriTable", MA_READ_WRITE, MF_COPY_HOST_PTR, AT_INT, 256*16, gTriTable);

  mTotalNumVertices     = GPGPUManager::GetSingleton().CreateBuffer(
                          "TotalNumVerts", MA_READ_WRITE, MF_COPY_HOST_PTR, AT_INT, 1, gTriTable);


  // temp for normals
  mpNormalVBO_debug     = GPGPUManager::GetSingleton().CreateBuffer(
                        "normalVBO_debug", MA_READ_WRITE, MF_NONE, AT_FLOAT4, 2500000, NULL);

}

void MarchingCubes::ShutdownBuffers()
{
  // Destroy all the buffers
  GPGPUManager::GetSingleton().ReleaseMemObject("NumVertsToGenerate");
  mpNumVertsToGenerate = NULL;
  
  GPGPUManager::GetSingleton().ReleaseMemObject("PosBufferOffset");
  mpPosBufferOffset = NULL;

  GPGPUManager::GetSingleton().ReleaseMemObject("ActiveVoxels");
  mpActiveVoxels = NULL;

  GPGPUManager::GetSingleton().ReleaseMemObject("EdgeTable");
  mpEdgeTable = NULL;
  
  GPGPUManager::GetSingleton().ReleaseMemObject("TriTable");
  mpTriTable = NULL;
  
  GPGPUManager::GetSingleton().ReleaseMemObject("TotalNumVerts");
  mTotalNumVertices = NULL;
}

void MarchingCubes::ResetBuffers(ZUInt32 volumeSize)
{
  // Shutdown the buffers
  ShutdownBuffers();

  // Recalculate sizes
  mVolumeSize = volumeSize;
  mNumVoxels  = mVolumeSize * mVolumeSize * mVolumeSize;

  InitializeBuffers();
}

// This polygonizes a whole block of size blockSize
void MarchingCubes::PolygonizeBlock(
      GPGPU::GPMemory* volumeData, GPGPU::GPMemory* posBuffer, GPGPU::GPMemory* normBuffer,
      GPGPU::GPMemory* texCoord01Buffer, float isoValue, ZUInt32 maxNumVertices)
{
  // Set params
  mVolumeData     = volumeData;
  mIsoValue       = isoValue;
  mMaxNumVertices = maxNumVertices;

  // Update the VBO pointers
  mpPositionVBO   = posBuffer;
  mpNormalVBO     = normBuffer;
  mpTexCoord01VBO = texCoord01Buffer;
  
  // Clear the buffers first
  ClearBuffers();

  // Classify the voxels 
  ClassifyVoxels();
  
  // Run a single thread to calculate the offsets  
  CalculateVboOffsets();

  // Generate the triangles
  if(mNumVertices < mMaxNumVertices)
    GenerateTriangles();
  else
    std::cout<<"$> Warning: VBO not big enough!\n";

  // temp -- read normal
  std::vector<Math::Vector4> normals(1000);
  GPGPUManager::GetSingleton().ReadBuffer(mpNormalVBO_debug, 0, 2, &normals[0]);

  //std::cout<<"";
}

void MarchingCubes::ClassifyVoxels()
{
  // # 1. ClassifyVoxels - get active (bool)
  GPKernel *krnlClassifyVoxel = mpProgram->GetKernel("ClassifyVoxels");

  krnlClassifyVoxel->SetArgument(0, mVolumeData);
  krnlClassifyVoxel->SetArgument(1, &mVolumeSize);
  krnlClassifyVoxel->SetArgument(2, &mIsoValue);
  krnlClassifyVoxel->SetArgument(3, mpActiveVoxels);
  krnlClassifyVoxel->SetArgument(4, mpNumVertsToGenerate);
  krnlClassifyVoxel->SetArgument(5, mpEdgeTable);
  krnlClassifyVoxel->SetArgument(6, mpTriTable);

  // Execute the kernels
  ZUInt32 workDimensions = 2;
  size_t lws[4] = { 1, 1, 0, 0 };
  size_t gws[4] = { VoxelHelper::GetSingleton().RoundUp(lws[0], mVolumeSize),
                    VoxelHelper::GetSingleton().RoundUp(lws[1], mVolumeSize),
                    0, 0 };
  
  GPGPUManager::GetSingleton().ExecuteKernel(krnlClassifyVoxel, workDimensions, NULL, gws, lws);

  //std::cout<<"$> ClassifyVoxels kernel executed\n";
}

void MarchingCubes::CalculateVboOffsets()
{
  // Run a single thread to calculate the vertex offsets for this block's voxels
  //     caclulate also the total number of vertices
  GPKernel *krnlCalcOffsets = mpProgram->GetKernel("CalculateVertBufferOffsets");

  krnlCalcOffsets->SetArgument(0, mpActiveVoxels);
  krnlCalcOffsets->SetArgument(1, mpNumVertsToGenerate);
  krnlCalcOffsets->SetArgument(2, mpPosBufferOffset);
  krnlCalcOffsets->SetArgument(3, &mNumVoxels);

  ZUInt32 workDimensions = 1;
  size_t gws[2] = { 1, 0 };
  size_t lws[2] = { 1, 0 };
  GPGPUManager::GetSingleton().ExecuteKernel(krnlCalcOffsets, 1, NULL, gws, lws);

  // Get total number of vertices to be generated
  // Find out how many vertices we will generates
  std::vector<ZUInt32> buff(mNumVoxels);
  GPGPUManager::GetSingleton().ReadBuffer(mpNumVertsToGenerate, 0, mNumVoxels, &buff[0]);
  
  std::vector<ZUInt32>::iterator itBuff;
  mNumVertices = 0;
  for(itBuff = buff.begin(); itBuff != buff.end(); itBuff++)
  {
    mNumVertices += (*itBuff);
  }
  std::cout<<"$> Number of vertices: " << mNumVertices << "\n";
  //\ Eof Calculate numVertices to generate


  //std::cout<<"$> CalculateVboOffsets kernel executed\n";
}

void MarchingCubes::GenerateTriangles()
{
  // # 2. GetGeneratedVertsOffsets
  GPKernel *krnlGenerateTriangles = mpProgram->GetKernel("GenerateTriangles");

  krnlGenerateTriangles->SetArgument(0, mVolumeData);
  krnlGenerateTriangles->SetArgument(1, &mVolumeSize);
  krnlGenerateTriangles->SetArgument(2, &mIsoValue);
  krnlGenerateTriangles->SetArgument(3, mpPositionVBO);
  krnlGenerateTriangles->SetArgument(4, mpNormalVBO);
  krnlGenerateTriangles->SetArgument(5, mpNumVertsToGenerate);
  krnlGenerateTriangles->SetArgument(6, mpPosBufferOffset);
  krnlGenerateTriangles->SetArgument(7, mpActiveVoxels);
  krnlGenerateTriangles->SetArgument(8, mpEdgeTable);
  krnlGenerateTriangles->SetArgument(9, mpTriTable);

  // Execute the kernel
  GPMemory* interopBuffers[] = { mpPositionVBO, mpNormalVBO };

  // Acquire the GL buffers
  GPGPUManager::GetSingleton().AcquireHardwareBuffer(1, interopBuffers);

  ZUInt32 workDimensions = 2;
  size_t lws[4] = { 1, 1, 0, 0 };
  size_t gws[4] = { VoxelHelper::GetSingleton().RoundUp(lws[0], mVolumeSize),
                    VoxelHelper::GetSingleton().RoundUp(lws[1], mVolumeSize),
                    0, 0 };

  GPGPUManager::GetSingleton().ExecuteKernel(krnlGenerateTriangles, workDimensions, NULL, gws, lws);

  // Release the GL buffers
  GPGPUManager::GetSingleton().ReleaseHardwareBuffer(1, interopBuffers);

  //std::cout<<"$> GenerateTriangles kernel executed\n";
}

void MarchingCubes::ClearBuffers()
{
  // Clears all buffers prior to marching cubes execution
  GPKernel *krnlClearBuffers = mpProgram->GetKernel("ClearBuffers");

  krnlClearBuffers->SetArgument(0,   mpNumVertsToGenerate);
  krnlClearBuffers->SetArgument(1,   mpPosBufferOffset);
  krnlClearBuffers->SetArgument(2,   mpActiveVoxels);
  krnlClearBuffers->SetArgument(3,   &mNumVoxels);

  size_t lws[2] = { 1, 0 };
  size_t gws[2] = {VoxelHelper::GetSingleton().RoundUp(lws[0], mNumVoxels), 0};

  GPGPUManager::GetSingleton().ExecuteKernel(krnlClearBuffers, 1, NULL, gws, lws);

  //std::cout<<"$> Clearing Marching cubes buffers...\n";
}