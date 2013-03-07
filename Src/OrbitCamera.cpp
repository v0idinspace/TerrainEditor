#include <OrbitCamera.h>



using namespace Zephyr;
using namespace Math;


OrbitCamera::OrbitCamera()
:	mType(CT_FIRST_PERSON),
  mPosition (Vector3(0, 0, 0)),
  mTarget(Vector3(0, 0, -1.0f)),
	mLookAt (Vector3(0, 0, 1.0f)),
	mRight (Vector3(1.0f, 0, 0)),
	mUp (Vector3(0, 1.0f, 0)),
	mbFrustumEnabled (false),
	mAspectRatio ((float)800/600),
	mFovAngle (ZEPHYR_MATH_PI/2),
	mNearPlane (1.0f),
	mFarPlane (1000.0f)
{
	// Initialize the orientation quaternion
  mOrientation = Quaternion::IDENTITY;

	// Default perspective projection matrix
	mProjectionMatrix.SetMatrixPerspectiveFovLH(mFovAngle, mAspectRatio, mNearPlane, mFarPlane);
	mInvProjectionMatrix	= mProjectionMatrix.Inverse();

  mAzimuth = 0;
  mPolar    = 0;
}

void OrbitCamera::Update()
{
	mLookAt.Normalize();
	mUp.Normalize();
	mRight.Normalize();

	// Transform LookAt vector by camera position
  //if(mType == CT_FIRST_PERSON)
	//  mTarget	= mLookAt + mPosition;

	// Reconstruct the Camera's view matrix
	mViewMatrix.SetViewMatrixLH(mPosition, mTarget, mUp);

	// Recalculte the Frustum
	if(mbFrustumEnabled)
	{
		//mFrustum.Calculate(5000.0f, mViewMatrix, mProjectionMatrix);
		mFrustum.Calculate(mFovAngle, mAspectRatio, mNearPlane, mFarPlane, mPosition, mTarget, mUp);
	}
	// ! DEBUG 
	//Math::DumpMatrix4(rotation);
	//Math::DumpVector3(lookAt);
     
  //mOrientation = Quaternion::IDENTITY;
}

void OrbitCamera::Shutdown()
{
	//return OK;
}

void OrbitCamera::Reset()
{
	mLookAt		= Vector3(0, 0, 1.0f);
	mRight		= Vector3(1.0f, 0, 0);
	mUp			  = Vector3(0, 1.0f, 0);
}

void OrbitCamera::Move(float distance)
{
  //if(mType == CT_FIRST_PERSON)
	  mPosition += mLookAt * distance;
    std::cout<<"";
}

void OrbitCamera::MovePerpendicular(float distanceX, float distanceY)
{
  if(mType == CT_FIRST_PERSON)
  {
	  mPosition += mRight * distanceX;
	  mPosition += mUp * distanceY;
  }
}

void OrbitCamera::RotateYaw(float angle)
{
  Quaternion orientation = Quaternion::IDENTITY;
  orientation.RotationAxisToQuaternion(angle, Vector3(0, 1.0f, 0));
	
  mLookAt	= orientation * mLookAt;
  mRight	= orientation * mRight;

}

void OrbitCamera::RotatePitch(float angle)
{
	Quaternion orientation = Quaternion::IDENTITY;
  orientation.RotationAxisToQuaternion(angle, mRight);

  mLookAt	= orientation * mLookAt;
  mUp		  = orientation * mUp;
}

void OrbitCamera::RotateRoll(float angle)
{

}

void OrbitCamera::OrbitRotate(float azimuth, float polar)
{
  Matrix4 rotX, rotY;
  rotX.SetRotationAroundAxis(polar, mRight);
  rotY.SetRotationAroundAxis(azimuth, mUp);

  mPosition = mPosition * (rotX * rotY);
  
  mLookAt = mTarget - mPosition;
  mRight = mRight * (rotX * rotY);
}

void OrbitCamera::OrbitRotateHorizontal(float angle)
{
  //mPolar += angle; 
}

void OrbitCamera::ZoomOnTarget(float distance)
{
  //mPosition += mLookAt * distance;
}