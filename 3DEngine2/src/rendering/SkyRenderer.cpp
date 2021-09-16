#include "pch.h"
#include "rendering/SkyRenderer.h"
#include "rendering/Camera.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "RenderingStateCache.h"

namespace tde 
{
	using namespace DirectX;

	SkyRenderer::SkyRenderer(ID3D11Device1* apDevice, std::shared_ptr<BaseCamera> apCamera, ID3D11Buffer** appLightBuffer,
		DirectX::CXMVECTOR aHeavenColor, DirectX::CXMVECTOR aHellColor)
		: mpCamera(apCamera)
		, mppLightBuffer(appLightBuffer)
	{
		mpVertexShader = VertexShaderCacheLocator::Get()->Get("SkyVS");
		mpPixelShader = PixelShaderCacheLocator::Get()->Get("SkyPS");
		mpSampler = SamplerCacheLocator::Get()->Get("linear");

		//	create vertex buffer
		{
			const float aspectRatio = apCamera->GetAspectRatio();
			const float t = std::tan(XMConvertToRadians(apCamera->GetVerticalFov() / 2.0f));
			const float nearZ = mpCamera->GetNearPlane();
			const float top = nearZ * t;
			const float bottom = -top;
			const float right = top * aspectRatio;
			const float left = -right;
			mVertices.clear();
			mVertices.push_back({ { left, top, nearZ }, { 0.0f, 0.0f } });
			mVertices.push_back({ { left, bottom, nearZ }, { 0.0f, 1.0f } });
			mVertices.push_back({ { right, bottom, nearZ }, { 1.0f, 1.0f } });
			mVertices.push_back({ { right, top, nearZ }, { 1.0f, 0.0f } });
			mVertices.push_back({ { left, top, nearZ }, { 0.0f, 0.0f } });
			mVertices.push_back({ { right, bottom, nearZ }, { 1.0f, 1.0f } });
		}
		D3D11_BUFFER_DESC vtxBufDesc;
		ZeroMemory(&vtxBufDesc, sizeof(D3D11_BUFFER_DESC));
		vtxBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vtxBufDesc.ByteWidth = 6 * sizeof(SkyVertex);
		vtxBufDesc.CPUAccessFlags = 0;
		vtxBufDesc.MiscFlags = 0;
		vtxBufDesc.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA vtxInitData;
		vtxInitData.pSysMem = &mVertices[0];
		vtxInitData.SysMemPitch = 0;
		vtxInitData.SysMemSlicePitch = 0;
		apDevice->CreateBuffer(&vtxBufDesc, &vtxInitData, mpVertexBuffer.ReleaseAndGetAddressOf());

		//	create vertex param buffer
		D3D11_BUFFER_DESC vtxParamBufDesc;
		ZeroMemory(&vtxParamBufDesc, sizeof(D3D11_BUFFER_DESC));
		vtxParamBufDesc.Usage = D3D11_USAGE_DEFAULT;
		vtxParamBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		vtxParamBufDesc.ByteWidth = sizeof(SkyVertexParams);
		vtxParamBufDesc.CPUAccessFlags = 0;
		apDevice->CreateBuffer(&vtxParamBufDesc, nullptr, mpVertexParamBuffer.ReleaseAndGetAddressOf());

		//	create sky parameters const buffer
		mSkyParams.mHeavenColor = aHeavenColor;
		mSkyParams.mHellColor = aHellColor;
		mSkyParams.mZNear = mpCamera->GetNearPlane();
		mSkyParams.mZFar = mpCamera->GetFarPlane();
		D3D11_BUFFER_DESC skyBufDesc;
		ZeroMemory(&skyBufDesc, sizeof(D3D11_BUFFER_DESC));
		skyBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		skyBufDesc.CPUAccessFlags = 0;
		skyBufDesc.Usage = D3D11_USAGE_DEFAULT;
		skyBufDesc.ByteWidth = sizeof(SkyParams);
		D3D11_SUBRESOURCE_DATA skyInitData;
		skyInitData.pSysMem = &mSkyParams;
		skyInitData.SysMemPitch = 0;
		skyInitData.SysMemSlicePitch = 0;
		apDevice->CreateBuffer(&skyBufDesc, &skyInitData, mpSkyParamBuffer.ReleaseAndGetAddressOf());
	}

	void SkyRenderer::Update(const float aDeltaTime)
	{
		//	update screen quad vertices according to camera position and orientation
		//XMVECTOR center = mpCamera->GetPosition() + mpCamera->GetNearPlane() * mpCamera->GetForwardVector();
		//XMVECTOR up = std::sin(mpCamera->GetVerticalFov()) * mpCamera->GetNearPlane() * mpCamera->GetUpVector();
		//XMVECTOR right = std::sin(mpCamera->GetVerticalFov()) * mpCamera->GetNearPlane() * mpCamera->GetAspectRatio() * mpCamera->GetRightVector();
		//XMFLOAT3 topLeft;
		//XMFLOAT3 topRight;
		//XMFLOAT3 bottomLeft;
		//XMFLOAT3 bottomRight;
		//XMStoreFloat3(&topLeft, center + up - right);
		//XMStoreFloat3(&topRight, center + up + right);
		//XMStoreFloat3(&bottomLeft, center - up - right);
		//XMStoreFloat3(&bottomRight, center - up + right);
		//mVertices.clear();
		//mVertices.emplace_back(topLeft, XMFLOAT2(0.0f, 0.0f));
		//mVertices.emplace_back(bottomLeft, XMFLOAT2(0.0f, 1.0f));
		//mVertices.emplace_back(bottomRight, XMFLOAT2(1.0f, 1.0f));
		//mVertices.emplace_back(topRight, XMFLOAT2(1.0f, 0.0f));
		//mVertices.emplace_back(topLeft, XMFLOAT2(0.0f, 0.0f));
		//mVertices.emplace_back(bottomRight, XMFLOAT2(1.0f, 1.0f));
	}

	void SkyRenderer::Render(ID3D11DeviceContext* apContext, ID3D11ShaderResourceView* apRawRenderTargetSRV, ID3D11ShaderResourceView* apDepthStencilSRV, const float aDeltaTime)
	{
		//	update vertex buffer
		//apContext->UpdateSubresource(mpVertexBuffer.Get(), 0, nullptr, &mVertices, 0, 0);
		SkyVertexParams vtxParams{ mpCamera->GetCameraWorldMatrix() };
		apContext->UpdateSubresource(mpVertexParamBuffer.Get(), 0, nullptr, &vtxParams, 0, 0);

		//	render
		//	IA
		UINT strides[] = { sizeof(SkyVertex) };
		UINT offsets[] = { 0 };
		mpVertexShader->SetInputLayout(apContext);
		apContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		apContext->IASetVertexBuffers(0, 1, mpVertexBuffer.GetAddressOf(), strides, offsets);
		//	VS
		apContext->VSSetShader(mpVertexShader->GetVertexShader(), nullptr, 0);
		apContext->VSSetConstantBuffers(0, 1, mpVertexParamBuffer.GetAddressOf());
		//	PS
		ID3D11Buffer* pPsConstBufs[2] = { *mppLightBuffer, mpSkyParamBuffer.Get() };
		ID3D11ShaderResourceView* pSRVs[2] = { apRawRenderTargetSRV, apDepthStencilSRV };
		apContext->PSSetShader(mpPixelShader->GetPixelShader(), nullptr, 0);
		apContext->PSSetSamplers(0, 1, mpSampler.GetAddressOf());
		apContext->PSSetConstantBuffers(0, 2, pPsConstBufs);
		apContext->PSSetShaderResources(0, 2, pSRVs);
		//	draw
		apContext->Draw(6, 0);
	}

	SkyRenderer::SkyVertex::SkyVertex(const DirectX::XMFLOAT3& aPosition, const DirectX::XMFLOAT2& aTexCoord)
		: mPosition(aPosition), mTexCoord(aTexCoord)
	{}
}
