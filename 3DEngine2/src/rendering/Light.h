#pragma once

namespace tde
{
#define MAX_LIGHTS 2

	enum class LightType
	{
		DIRECTIONAL		= 1,
		POINT			= 2,
		SPOT			= 3
	};

	struct alignas(16) LightSource
	{
		DirectX::XMVECTOR mPosition;
		DirectX::XMVECTOR mDirection;
		DirectX::XMVECTOR mColor;
		float mIntensity;
		float mLinearAttenuation;
		float mQuadraticAttenuation;
		int mType;	//	1: directional  2: point  3: spot
		int mIsEnabled;
		int mPadding[3];
	};

	struct alignas(16) Lights
	{
		DirectX::XMVECTOR mEyePosition;
		LightSource mLights[MAX_LIGHTS];
	};
}