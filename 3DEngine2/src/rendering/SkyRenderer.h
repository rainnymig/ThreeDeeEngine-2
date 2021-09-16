#pragma once

namespace tde 
{
	class BaseCamera;
	class VertexShader;
	class PixelShader;

	class SkyRenderer
	{
	public:

		struct alignas(16) SkyParams 
		{
			DirectX::XMVECTOR mHeavenColor;
			DirectX::XMVECTOR mHellColor;
			float mZNear;
			float mZFar;
			float mPadding[2];
		};

		struct alignas(16) SkyVertexParams
		{
			DirectX::XMMATRIX mCameraWorldMatrix;
		};

		struct SkyVertex
		{
			SkyVertex(const DirectX::XMFLOAT3& aPosition, const DirectX::XMFLOAT2& aTexCoord);
			DirectX::XMFLOAT3 mPosition;
			DirectX::XMFLOAT2 mTexCoord;
		};

		SkyRenderer(ID3D11Device1* apDevice, std::shared_ptr<BaseCamera> apCamera, ID3D11Buffer** appLightBuffer,
			DirectX::CXMVECTOR aHeavenColor, DirectX::CXMVECTOR aHellColor);
		void Update(const float aDeltaTime);
		void Render(ID3D11DeviceContext* apContext, ID3D11ShaderResourceView* apRawRenderTargetSRV, ID3D11ShaderResourceView* apDepthStencilSRV, const float aDeltaTime);

	private:
		std::vector<SkyVertex> mVertices;
		std::shared_ptr<BaseCamera> mpCamera;
		std::shared_ptr<VertexShader> mpVertexShader;
		std::shared_ptr<PixelShader> mpPixelShader;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mpSampler;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpVertexParamBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpSkyParamBuffer;
		ID3D11Buffer** mppLightBuffer;
		SkyParams mSkyParams;
	};
}