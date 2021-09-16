#include "pch.h"
#include "rendering/Camera.h"


namespace tde
{
	using namespace DirectX;

	XMVECTOR XM_CALLCONV SmoothDamp(FXMVECTOR aCurrent, FXMVECTOR aTarget, XMVECTOR& aOutVelocity, const float aSmoothTime, const float aDeltaTime)
	{
		//	from Game Programming Gems 4 1.10
		float omega = 2.0f / aSmoothTime;
		float x = omega * aDeltaTime;
		float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
		XMVECTOR change = aCurrent - aTarget;
		XMVECTOR temp = (aOutVelocity + omega * change) * aDeltaTime;
		aOutVelocity = (aOutVelocity - omega * temp) * exp;
		return aTarget + (change + temp) * exp;
	}

	XMMATRIX SimpleCamera::GetViewMatrix()
	{
		return XMMatrixLookAtLH({ 0.0f, 5.0f, -10.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, {0.0f, 1.0f, 0.0f, 0.0f});
	}

	XMMATRIX SimpleCamera::GetProjectionMatrix()
	{
		float aspectRatio = 16.0f / 9.0f;
		float verticalFov = 45.0f;
		float nearPlane = 1.0f;
		float farPlane = 100.0f;

		return XMMatrixPerspectiveFovLH(XMConvertToRadians(verticalFov), aspectRatio, nearPlane, farPlane);
	}

	DirectX::XMMATRIX SimpleCamera::GetCameraWorldMatrix()
	{
		return XMMatrixInverse(nullptr, GetViewMatrix());
	}

	XMMATRIX BaseCamera::GetViewMatrix()
	{
		if (mIsTransformDirty)
		{
			XMMATRIX rotationMatrix = XMMatrixTranspose(XMMatrixRotationQuaternion(mRotation));
			XMMATRIX translationMatrix = XMMatrixTranslationFromVector(-mPosition);
			mViewMatrix = translationMatrix * rotationMatrix;
			mIsTransformDirty = false;
		}

		return mViewMatrix;
	}

	XMMATRIX BaseCamera::GetProjectionMatrix()
	{
		if (mIsProjectionDirty)
		{
			mProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(mVerticalFov), mAspectRatio, mNearPlane, mFarPlane);
			mIsProjectionDirty = false;
		}
		return mProjectionMatrix;
	}

	DirectX::XMMATRIX BaseCamera::GetCameraWorldMatrix()
	{
		return XMMatrixInverse(nullptr, GetViewMatrix());
	}

	void BaseCamera::Update(const float aDeltaTime)
	{
	}

	XMVECTOR BaseCamera::GetForwardVector() const
	{
		XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		return XMVector3Rotate(forward, mRotation);
	}

	XMVECTOR BaseCamera::GetRightVector() const
	{
		XMVECTOR right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		return XMVector3Rotate(right, mRotation);
	}

	XMVECTOR BaseCamera::GetUpVector() const
	{
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		return XMVector3Rotate(up, mRotation);
	}

	FreeFlightCamera::FreeFlightCamera(HWND aWindowHandle)
	{
		mpMouse = std::make_unique<Mouse>();
		mpKeyboard = std::make_unique<Keyboard>();
		mKeys.Reset();
		mMouseButtons.Reset();
		mpMouse->SetWindow(aWindowHandle);
	}

	void FreeFlightCamera::Update(const float aDeltaTime)
	{
		static constexpr float CAMERA_ROTATION_SPEED = XMConvertToRadians(25.0f);
		static constexpr float CAMERA_MOVE_SPEED_SLOW = 2.0f;
		static constexpr float CAMERA_MOVE_SPEED_NORMAL = 8.0f;
		static constexpr float CAMERA_MOVE_SPEED_FAST = 20.0f;

		static XMVECTOR currentVelocity = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		static XMVECTOR targetVelocity = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		static XMVECTOR currentAcceleration = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		auto keyboard = mpKeyboard->GetState();
		auto mouse = mpMouse->GetState();
		mKeys.Update(keyboard);
		mMouseButtons.Update(mouse);
		
		if (mouse.positionMode == Mouse::MODE_RELATIVE)
		{
			float invertY = mInvertY ? -1.0f : 1.0f;
			float invertX = mInvertX ? -1.0f : 1.0f;
			float pitchDelta = invertY * static_cast<float>(mouse.y) * CAMERA_ROTATION_SPEED * aDeltaTime;
			float yawDelta = invertX * static_cast<float>(mouse.x) * CAMERA_ROTATION_SPEED * aDeltaTime;

			mPitch += pitchDelta;
			mYaw += yawDelta;

			SetRotation(XMQuaternionRotationRollPitchYaw(mPitch, mYaw, 0.0f));
		}

		if (mKeys.pressed.V)
		{
			mpMouse->SetMode(mouse.positionMode == Mouse::MODE_ABSOLUTE ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
		}

		float movementX = static_cast<int>(keyboard.D || keyboard.Right) - static_cast<int>(keyboard.A || keyboard.Left);
		float movementZ = static_cast<int>(keyboard.W || keyboard.Up) - static_cast<int>(keyboard.S || keyboard.Down);
		float movementY = static_cast<int>(keyboard.Z || keyboard.PageUp) - static_cast<int>(keyboard.C || keyboard.PageDown);
		targetVelocity = XMVectorSet(movementX, movementY, movementZ, 0.0f) * CAMERA_MOVE_SPEED_NORMAL;

		currentVelocity = SmoothDamp(currentVelocity, targetVelocity, currentAcceleration, 0.1f, aDeltaTime);

		SetPosition(mPosition + SimpleMath::Vector4(XMVector3Rotate(currentVelocity * aDeltaTime, mRotation)));
	}

}
