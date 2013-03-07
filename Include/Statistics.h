/**
----------------------------------------------------------------------------
	@File:			Statistics.h
   
	System:         SCTGame Engine 
	Component Name:	SCT Statistics
	Status:         Version 1.0 Release 1 

	Language: C++
   
	License: GNU Public License	[*!]
	Licensed Material - Property of ...
   
	Author: Anastasios 'v0id' Giannakopoulos
	E-Mail: dotvoidd@gmail.com
   
	Description:	Header file for Project X
					This file contains the defined types for Project X
					This is sometimes called the "Abstract" and may be
					followed by a section called "Notes".
 
	Limitations:
   
	Function:

----------------------------------------------------------------------------
*/


#ifndef SCT_STATISTICS_H
#define SCT_STATISTICS_H


#include <VoxelHeaders.h>


using namespace Zephyr;


// Class name:	Statistics
// Description: provides SCTEngine statistics (framerate counter, cpu usage, etc)
// ...
class Statistics : public Core::Singleton<Statistics>
{
public:
	Statistics()
    : mFps(0), 
      mMsPerFrame(0), 
      mFrameCnt(0), 
      t_base(0), 
      mCounterInterval(1),
      mNumTriangles(0),
      mFpsStats(true), 
      mCpuStats (true)
  {}
	~Statistics() { Shutdown(); }

	void				  Initialize();
	void				  Update();
	void				  Shutdown();

	// Get, Set Functions
	void		      SetFpsCounterInterval(ZUInt32 CounterInterval) 
  { mCounterInterval = CounterInterval; }
	void					GetFrameStats(float &Fps, float &MsPFrame);
	void					GetCpuPercentage(ZUInt32 &cpuPercentage);

	void		      EnableFpsStats(bool Mode) { mFpsStats	= Mode; }
	void		      EnableCPUStats(bool Mode) { mCpuStats	= Mode; }

  void          SetNumTriangles(ZUInt32 numTriangles) { mNumTriangles = numTriangles; }
  ZUInt32       GetNumTriangles() { return mNumTriangles; }

private:
	// --- Private Variables
	// Frame rate counter
	float					mFps;
	float					mMsPerFrame;
	ZUInt64				mFrameCnt;
	double				t_base;

	ZUInt32				mCounterInterval;

	bool					mFpsStats,
							  mCpuStats;

	// CPU stats
	bool					mCanReadCpu;
	HQUERY				mQueryHandle;
	HCOUNTER			mCounterHandle;

	ZUInt64				mLastSampleTime,
							  mCpuUsage;

  // Render stats
  ZUInt32       mNumTriangles;

	// --- Private Functions
	void					CalculateFpsStats();
	void					CalculateCpuStats();
};

// Singlenton
template<> Statistics* Core::Singleton<Statistics>::msSingleton = 0;


#endif