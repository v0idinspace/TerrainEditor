#include <VoxelHelper.h>
//#include <GPGPU\ZephyrGPGPUMemory.h>


using namespace Zephyr;
using namespace GPGPU;


VoxelHelper::VoxelHelper()
  : mpProgram(NULL),
    mpScanBuffer(NULL),
    mWorkGroupSize(256)
{
  Initialize();
}

VoxelHelper::~VoxelHelper()
{
  Shutdown();
}

void VoxelHelper::Initialize()
{
}

void VoxelHelper::Shutdown()
{
  // Cleanup
}

//  Round up to the nearest multiple of the group size
size_t VoxelHelper::RoundUp(int groupSize, int globalSize)
{
  int r = globalSize % groupSize;

  if(r == 0)
    return globalSize;
  else
    return globalSize + groupSize - r;
}