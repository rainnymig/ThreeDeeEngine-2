#pragma once
#include "rendering/Light.h"

namespace tde
{
	class IGameObject;
	class ICamera;
	class BaseCamera;
	class VertexShader;
	class PixelShader;

	class Scene
	{
	public:

		void Init(ID3D11Device1* apDevice, HWND aWindowHandle);
		void Update(const float aDeltaTime);
		void Render(ID3D11DeviceContext1* apContext, const float aDeltaTime);
		void Destroy();

	private:

		Lights mLights;
		
		std::vector<std::shared_ptr<IGameObject>> mGameObjects;
		std::shared_ptr<BaseCamera> mpCamera;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpLightBuffer;

		HRESULT PrivCreateLightBuffer(ID3D11Device* apDevice);
		void PrivUpdateLights(ID3D11DeviceContext1* apContext, const float aDeltaTime);
	};
}