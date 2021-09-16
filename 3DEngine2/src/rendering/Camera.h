#pragma once

namespace tde
{
	class ICamera
	{
	public:
		virtual DirectX::XMMATRIX GetViewMatrix() = 0;
		virtual DirectX::XMMATRIX GetProjectionMatrix() = 0;
		virtual DirectX::XMMATRIX GetCameraWorldMatrix() = 0;
	};

	class SimpleCamera : public ICamera
	{
	public:
		virtual DirectX::XMMATRIX GetViewMatrix() override;
		virtual DirectX::XMMATRIX GetProjectionMatrix() override;
		virtual DirectX::XMMATRIX GetCameraWorldMatrix() override;
	};

	class BaseCamera : public ICamera
	{
	public:
		virtual DirectX::XMMATRIX GetViewMatrix() override;
		virtual DirectX::XMMATRIX GetProjectionMatrix() override;
		virtual DirectX::XMMATRIX GetCameraWorldMatrix() override;

		virtual void Update(const float aDeltaTime);

		inline void XM_CALLCONV SetPosition(DirectX::FXMVECTOR aPosition)
		{ 
			mPosition = aPosition; 
			mIsTransformDirty = true; 
		}
		inline void XM_CALLCONV SetRotation(DirectX::FXMVECTOR aRotation)
		{ 
			mRotation = aRotation; 
			mIsTransformDirty = true;
		}
		inline void SetAspectRatio(const float aAspectRatio)
		{
			mAspectRatio = aAspectRatio;
			mIsProjectionDirty = true;
		}
		inline void SetNearPlane(const float aNearPlane)
		{
			mNearPlane = aNearPlane;
			mIsProjectionDirty = true;
		}
		inline void SetFarPlane(const float aFarPlane)
		{
			mFarPlane = aFarPlane;
			mIsProjectionDirty = true;
		}
		//	set vertical fov in angle
		inline void SetVerticalFov(const float aVerticalFov)
		{
			mVerticalFov = aVerticalFov;
			mIsProjectionDirty = true;
		}
		inline DirectX::XMVECTOR GetPosition() const { return mPosition; }
		inline DirectX::XMVECTOR GetRotation() const { return mRotation; }
		inline float GetAspectRatio() const { return mAspectRatio; }
		inline float GetNearPlane() const { return mNearPlane; }
		inline float GetFarPlane() const { return mFarPlane; }
		//	get vertical fov in angle
		inline float GetVerticalFov() const { return mVerticalFov; }		

		DirectX::XMVECTOR GetForwardVector() const;
		DirectX::XMVECTOR GetRightVector() const;
		DirectX::XMVECTOR GetUpVector() const;

	protected:
		DirectX::SimpleMath::Matrix mViewMatrix;
		DirectX::SimpleMath::Matrix mProjectionMatrix;

		DirectX::SimpleMath::Vector4 mPosition = { 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::SimpleMath::Vector4 mRotation = { 0.0f, 0.0f, 0.0f, 0.0f }; //	quaternion
		
		float mAspectRatio = 16.0f / 9.0f;
		float mNearPlane = 1.0f;
		float mFarPlane = 100.0f;
		float mVerticalFov = 45.0f;		//	angle
		
		bool mIsTransformDirty = true;
		bool mIsProjectionDirty = true;
	};

	class FreeFlightCamera : public BaseCamera
	{
	public:
		FreeFlightCamera(HWND aWindowHandle);
		virtual void Update(const float aDeltaTime) override;
		inline void SetPitch(const float aPitch) { mPitch = aPitch; }
		inline void SetYaw(const float aYaw) { mYaw = aYaw; }
		inline float GetPitch() const { return mPitch; }
		inline float GetYaw() const { return mYaw; }
		inline void SetInvertX(const bool aInvertX) { mInvertX = aInvertX; }
		inline void SetInvertY(const bool aInvertY) { mInvertY = aInvertY; }
		inline bool GetInvertX() const { return mInvertX; }
		inline bool GetInvertY() const { return mInvertY; }
	private:
		DirectX::Keyboard::KeyboardStateTracker mKeys;
		DirectX::Mouse::ButtonStateTracker mMouseButtons;
		std::unique_ptr<DirectX::Mouse> mpMouse;
		std::unique_ptr<DirectX::Keyboard> mpKeyboard;
		float mPitch = 0.0f;
		float mYaw = 0.0f;
		bool mInvertX = false;
		bool mInvertY = true;
	};
}