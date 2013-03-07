/**
----------------------------------------------------------------------------
	@File:			OrbitCamera.h
   
	System:         SCTGame Engine 
	Component Name:	SCT Camera 
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

#ifndef _ORBIT_CAMERA_H_
#define _ORBIT_CAMERA_H_


#include <Core\ZephyrHeaders.h>
#include <Math\ZephyrMatrix4.h>
#include <Math\ZephyrVector3.h>
#include <Math\ZephyrQuaternion.h>
#include <Math\ZephyrFrustum.h>
#include <Core\ZephyrSingleton.h>


enum CameraType
{
  CT_FIRST_PERSON,
  CT_ORBIT
};

namespace Zephyr
{
  
  /// Class OrbitCamera
	//  ...
	class OrbitCamera : public Core::Singleton<OrbitCamera>
	{
	public:
		OrbitCamera();
		virtual				~OrbitCamera() { Shutdown(); };

		void					Update();
		void					Shutdown();
		void					Reset();

		// Camera functions
		void						Move(float distance);
		void						MovePerpendicular(float distanceX, float distanceY);
		void						RotateYaw(float angle);
		void						RotatePitch(float angle);
		void						RotateRoll(float angle);

    // Orbit camera controlls
    void            OrbitRotate(float azimuth, float polar);
    void            OrbitRotateHorizontal(float angle);
    void            ZoomOnTarget(float distance);

		inline void			UseFrustum(bool mode)
		{
			mbFrustumEnabled = mode;
		}

		inline bool			IsFrustumEnabled()
		{
			return mbFrustumEnabled;
		}

		// Set, Get Functions
		inline void			SetPosition(const Math::Vector3 &position)
		{
			mPosition	= position;
		}

		inline void			SetPosition(float x, float y, float z)
		{
			mPosition	= Math::Vector3(x, y, z);
		}

		inline void			SetProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane)
		{
			mProjectionMatrix.SetMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
			mInvProjectionMatrix	= mProjectionMatrix.Inverse();

			mAspectRatio	= aspectRatio;
			mFovAngle		= fov;
			mNearPlane		= nearPlane;
			mFarPlane		= farPlane;
		}

		inline Math::Matrix4&		GetViewMatrix()
		{
			return mViewMatrix;
		}

		inline Math::Matrix4&		GetProjectionMatrix()
		{
			return mProjectionMatrix;
		}

		inline Math::Matrix4&		GetInverseProjectionMatrix()
		{
			return mInvProjectionMatrix;
		}

		inline Math::Vector3&		GetPosition()
		{
			return mPosition;
		}

		inline Math::Quaternion&	GetRotation()
		{
			return mOrientation;
		}

		inline Math::Frustum&		GetFrustum()
		{
			return mFrustum;
		}

    void SetType(CameraType type) { mType = type;}
    void SetTarget(Math::Vector3 &target) { mTarget = target; }

    // temp
    		Math::Vector3			mLookAt,
								      mRight,
								      mUp;

	protected:

	private:
		Math::Quaternion	mOrientation;			// Will hold the rotation
		Math::Matrix4			mViewMatrix,
								      mProjectionMatrix,
								      mInvProjectionMatrix;

		// Camera Properties
    CameraType        mType;
		Math::Vector3			mPosition;
    Math::Vector3     mTarget;
			


		// Frustum
		Math::Frustum			mFrustum;
		bool					    mbFrustumEnabled;
		float					    mAspectRatio,
								      mFovAngle,
								      mNearPlane,
								      mFarPlane;
			
    // Orbit camera
    float mAzimuth,
          mPolar;
			

		float	yaw, pitch;
	};

}

// Singleton
// temp
template<> Zephyr::OrbitCamera* Zephyr::Core::Singleton<Zephyr::OrbitCamera>::msSingleton = 0;

#endif