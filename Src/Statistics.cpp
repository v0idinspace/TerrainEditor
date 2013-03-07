#include <Statistics.h>
#include <Core\Win32\ZephyrWin32Timer.h>


using namespace Zephyr;
using namespace Core;


void Statistics::Initialize()
{
	PDH_STATUS status;

	// Initialize the flag indicating whether this object can read the system cpu usage or not.
	mCanReadCpu = true;

	// Create a query object to poll cpu usage.
	status = PdhOpenQuery(NULL, 0, &mQueryHandle);
	if(status != ERROR_SUCCESS)
		mCanReadCpu = false;

	// Set query object to poll all cpus in the system.
	status = PdhAddCounter(mQueryHandle, TEXT("\\Processor(_Total)\\% processor time"), 0, &mCounterHandle);
	if(status != ERROR_SUCCESS)
		mCanReadCpu = false;

	mLastSampleTime = GetTickCount(); 

	mCpuUsage = 0;
}

void Statistics::Update()
{
	// Update Fps counter
	if(mFpsStats)
		CalculateFpsStats();

	if(mCpuStats)
		CalculateCpuStats();
}

void Statistics::Shutdown()
{
	if(mCanReadCpu)
		PdhCloseQuery(mQueryHandle);
}

void Statistics::CalculateFpsStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.
	mFrameCnt++;

  float gameTime = Timer::GetSingleton().GetGameTime();
	ZUInt32 cpu;
	GetCpuPercentage(cpu);

	// Compute averages over one second period.
	if((gameTime - t_base) >= mCounterInterval)
	{
		mFps		= (float) (mFrameCnt / mCounterInterval);
		mMsPerFrame	= 1000.0f / mFps;

		//std::cout << "> Fps = " << mFps << ", Ms per frame = " << mMsPerFrame <<  ", Cpu = " << cpu << "% \n";

		mFrameCnt	= 0;
		t_base		+= mCounterInterval;
	}
}
			
void Statistics::CalculateCpuStats()
{
	PDH_FMT_COUNTERVALUE value; 

	if(mCanReadCpu)
	{
		if((mLastSampleTime + 1000) < GetTickCount())
		{
			mLastSampleTime = GetTickCount(); 

			PdhCollectQueryData(mQueryHandle);
        
			PdhGetFormattedCounterValue(mCounterHandle, PDH_FMT_LONG, NULL, &value);

			mCpuUsage = value.longValue;
		}
	}
}

void Statistics::GetFrameStats(float &Fps, float &MsPFrame)
{
	Fps			= mFps;
	MsPFrame	= mMsPerFrame;
}

void Statistics::GetCpuPercentage(ZUInt32 &cpuPercentage)
{
	if(mCanReadCpu)
		cpuPercentage = (int)mCpuUsage;
	else
		cpuPercentage = 0;
}