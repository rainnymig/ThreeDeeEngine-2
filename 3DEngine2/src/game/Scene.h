#pragma once
#include "rendering/Light.h"

namespace tde
{
	class IGameObject;
	class ICamera;
	class BaseCamera;
	class VertexShader;
	class PixelShader;
	class SkyRenderer;
	class CubeWorldRenderer;

	class Scene
	{
	public:

		void Init(ID3D11Device1* apDevice, HWND aWindowHandle);
		void Update(ID3D11Device* apDevice, const float aDeltaTime);
		void Render(ID3D11Device* apDevice, ID3D11DeviceContext1* apContext, const float aDeltaTime);
		void PostProcess(ID3D11DeviceContext1* apContext, 
			ID3D11ShaderResourceView* apRawRenderTargetSRV, 
			ID3D11ShaderResourceView* apDepthStencilSRV, 
			const float aDeltaTime);
		void Destroy();


		void OnScreenSizeChange(int aWidth, int aHeight);

	private:

		Lights mLights;
		
		std::vector<std::shared_ptr<IGameObject>> mGameObjects;
		std::shared_ptr<SkyRenderer> mpSkyRenderer;
		std::shared_ptr<CubeWorldRenderer> mpCubeWorldRenderer;
		std::shared_ptr<BaseCamera> mpCamera;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpLightBuffer;

		HRESULT PrivCreateLightBuffer(ID3D11Device* apDevice);
		void PrivUpdateLights(ID3D11DeviceContext1* apContext, const float aDeltaTime);
	};
}