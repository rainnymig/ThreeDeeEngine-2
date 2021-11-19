#include "pch.h"
#include "game/Scene.h"

#include "game/GameObject.h"
#include "rendering/Camera.h"
#include "common/ServiceLocator.h"
#include "common/BaseCache.h"
#include "rendering/VertexShader.h"
#include "rendering/PixelShader.h"
#include "rendering/RenderingStateCache.h"
#include "rendering/SkyRenderer.h"
#include "rendering/CubeWorldRenderer.h"

namespace tde
{
	using namespace DirectX;

	void Scene::Init(ID3D11Device1* apDevice, HWND aWindowHandle)
	{
		//	load shaders
		const D3D11_INPUT_ELEMENT_DESC meshVertexLayout[] =
		{
			{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = sizeof(meshVertexLayout) / sizeof(meshVertexLayout[0]);
		std::shared_ptr<VertexShader> pBasicVS = std::make_shared<VertexShader>(L"shaders/BasicVS.cso", &meshVertexLayout[0], numElements, apDevice);
		if (pBasicVS)
		{
			VertexShaderCacheLocator::Get()->InsertIfNotExists("BasicVS", pBasicVS);
		}
		const D3D11_INPUT_ELEMENT_DESC skyVertexLayout[] =
		{
			{"POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		numElements = sizeof(skyVertexLayout) / sizeof(skyVertexLayout[0]);
		std::shared_ptr<VertexShader> pSkyVS = std::make_shared<VertexShader>(L"shaders/SkyVS.cso", &skyVertexLayout[0], numElements, apDevice);
		if (pSkyVS)
		{
			VertexShaderCacheLocator::Get()->InsertIfNotExists("SkyVS", pSkyVS);
		}
		const D3D11_INPUT_ELEMENT_DESC boxVertexLayout[] =
		{
			{"POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD",		0, DXGI_FORMAT_R32_UINT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		numElements = sizeof(boxVertexLayout) / sizeof(boxVertexLayout[0]);
		std::shared_ptr<VertexShader> pBoxVS = std::make_shared<VertexShader>(L"shaders/BoxVS.cso", &boxVertexLayout[0], numElements, apDevice);
		if (pBoxVS)
		{
			VertexShaderCacheLocator::Get()->InsertIfNotExists("BoxVS", pBoxVS);
		}

		std::shared_ptr<PixelShader> pPhongPS = std::make_shared<PixelShader>(L"shaders/PhongPS.cso", apDevice);
		if (pPhongPS)
		{
			PixelShaderCacheLocator::Get()->InsertIfNotExists("PhongPS", pPhongPS);
		}
		std::shared_ptr<PixelShader> pSkyPS = std::make_shared<PixelShader>(L"shaders/SkyPS.cso", apDevice);
		if (pSkyPS)
		{
			PixelShaderCacheLocator::Get()->InsertIfNotExists("SkyPS", pSkyPS);
		}
		std::shared_ptr<PixelShader> pBoxPS = std::make_shared<PixelShader>(L"shaders/BoxPS.cso", apDevice);
		if (pBoxPS)
		{
			PixelShaderCacheLocator::Get()->InsertIfNotExists("BoxPS", pBoxPS);
		}

		//	create samplers
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pLinearSampler;
		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 16;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.BorderColor[1] = 1.0f;
		samplerDesc.BorderColor[2] = 1.0f;
		samplerDesc.BorderColor[3] = 1.0f;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.MaxLOD = FLT_MAX;
		apDevice->CreateSamplerState(&samplerDesc, pLinearSampler.ReleaseAndGetAddressOf());
		if (pLinearSampler)
		{
			SamplerCacheLocator::Get()->InsertIfNotExists("linear", pLinearSampler);
		}

		//	create rasterizer state
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterState;
		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
		rasterizerDesc.AntialiasedLineEnable = FALSE;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = TRUE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.ScissorEnable = FALSE;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		apDevice->CreateRasterizerState(&rasterizerDesc, pRasterState.ReleaseAndGetAddressOf());
		if (pRasterState)
		{
			RasterizerStateCacheLocator::Get()->InsertIfNotExists("normal", pRasterState);
		}

		//	create depth stencil state
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDepthEnableState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDepthDisableState;
		D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
		ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		depthStencilStateDesc.DepthEnable = TRUE;
		depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilStateDesc.StencilEnable = FALSE;
		apDevice->CreateDepthStencilState(&depthStencilStateDesc, pDepthEnableState.ReleaseAndGetAddressOf());
		if (pDepthEnableState)
		{
			DepthStencilStateCacheLocator::Get()->InsertIfNotExists("depthEnableStencilDisable", pDepthEnableState);
		}
		depthStencilStateDesc.DepthEnable = FALSE;
		depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depthStencilStateDesc.StencilEnable = FALSE;
		apDevice->CreateDepthStencilState(&depthStencilStateDesc, pDepthDisableState.ReleaseAndGetAddressOf());
		if (pDepthDisableState)
		{
			DepthStencilStateCacheLocator::Get()->InsertIfNotExists("depthDisableStencilDisable", pDepthDisableState);
		}

		//	create blend state
		Microsoft::WRL::ComPtr<ID3D11BlendState> pBlendState;
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		apDevice->CreateBlendState(&blendDesc, pBlendState.ReleaseAndGetAddressOf());
		if (pBlendState)
		{
			BlendStateCacheLocator::Get()->InsertIfNotExists("normal", pBlendState);
		}

		//	create camera
		RECT windowRect;
		mpCamera = std::make_shared<FreeFlightCamera>(aWindowHandle);
		mpCamera->SetPosition({ 0, 2, -10, 1 });
		if (GetWindowRect(aWindowHandle, &windowRect))
		{
			float width = windowRect.right - windowRect.left;
			float height = windowRect.bottom - windowRect.top;
			mpCamera->SetAspectRatio(width / height);
		}
		else
		{
			mpCamera->SetAspectRatio(16.0f / 9.0f);
		}

		//	prepare lights
		mLights.mLights[0].mIsEnabled = true;
		mLights.mLights[0].mType = static_cast<int>(LightType::DIRECTIONAL);
		mLights.mLights[0].mColor = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		mLights.mLights[0].mDirection = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		mLights.mLights[0].mIntensity = 1.0f;

		mLights.mLights[1].mIsEnabled = true;
		mLights.mLights[1].mType = static_cast<int>(LightType::DIRECTIONAL);
		mLights.mLights[1].mColor = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		mLights.mLights[1].mDirection = DirectX::XMVectorSet(-1.0f, -4.0f, 1.0f, 0.0f);
		mLights.mLights[1].mIntensity = 0.5f;

		mLights.mLights[2].mIsEnabled = false;
		mLights.mLights[3].mIsEnabled = false;

		PrivCreateLightBuffer(apDevice);

		//	spawn game objects
		std::shared_ptr<SimpleModelGameObject> go = std::make_shared<SimpleModelGameObject>();
		go->Init("Fortnite-Plane.fbx", 
			apDevice, 
			mpCamera, 
			VertexShaderCacheLocator::Get()->Get("BasicVS"), 
			PixelShaderCacheLocator::Get()->Get("PhongPS"),
			mpLightBuffer.GetAddressOf());
		mGameObjects.emplace_back(go);

		//	create sky renderer
		DirectX::XMVECTOR heavenColor = XMVectorSet(0.8353f, 0.9412f, 0.9804f, 1.0f);
		DirectX::XMVECTOR hellColor = XMVectorSet(0.7980f, 0.7980f, 0.7980f, 1.0f);
		mpSkyRenderer = std::make_shared<SkyRenderer>(apDevice, mpCamera, mpLightBuffer.GetAddressOf(), heavenColor, hellColor);

		//	create cube world renderer
		std::shared_ptr<CubeWorld> cubeWorld = createCubeWorldFromBinaryFile("test_cube_world", 8, 8, 8);
		mpCubeWorldRenderer = std::make_shared<CubeWorldRenderer>(apDevice, cubeWorld, mpCamera, mpLightBuffer.GetAddressOf());
		mpCubeWorldRenderer->SetPosition({ 0.0f, 0.0, 10.0f, 1.0f });
		mpCubeWorldRenderer->SetScale(1.0f);
		mpCubeWorldRenderer->UpdateBuffer(apDevice);
	}

	void Scene::Update(ID3D11Device* apDevice, const float aDeltaTime)
	{
		mpCamera->Update(aDeltaTime);
		for (auto pGameObjects : mGameObjects)
		{
			pGameObjects->Update(aDeltaTime);
		}
		mpSkyRenderer->Update(aDeltaTime);
	}

	void Scene::Render(ID3D11Device* apDevice, ID3D11DeviceContext1* apContext, const float aDeltaTime)
	{
		PrivUpdateLights(apContext, aDeltaTime);

		apContext->OMSetDepthStencilState(DepthStencilStateCacheLocator::Get()->Get("depthEnableStencilDisable").Get(), 1);
		apContext->OMSetBlendState(BlendStateCacheLocator::Get()->Get("normal").Get(), nullptr, 0xffffffff);
		apContext->RSSetState(RasterizerStateCacheLocator::Get()->Get("normal").Get());

		for (auto pGameObjects : mGameObjects)
		{
			pGameObjects->Render(apContext, aDeltaTime);
		}

		mpCubeWorldRenderer->Render(apContext, aDeltaTime);
	}

	void Scene::PostProcess(ID3D11DeviceContext1* apContext, ID3D11ShaderResourceView* apRawRenderTargetSRV, ID3D11ShaderResourceView* apDepthStencilSRV, const float aDeltaTime)
	{
		mpSkyRenderer->Render(apContext, apRawRenderTargetSRV, apDepthStencilSRV, aDeltaTime);
	}

	void Scene::Destroy()
	{
		for (auto pGameObjects : mGameObjects)
		{
			pGameObjects->Destroy();
		}
		mGameObjects.clear();
		mpSkyRenderer.reset();
		mpCamera.reset();
		SAFE_RELEASE(mpLightBuffer);
	}

	void Scene::OnScreenSizeChange(int aWidth, int aHeight)
	{
		mpCamera->SetAspectRatio(static_cast<float>(aWidth) / static_cast<float>(aHeight));
	}

	HRESULT Scene::PrivCreateLightBuffer(ID3D11Device* apDevice)
	{
		D3D11_BUFFER_DESC bufDesc;
		ZeroMemory(&bufDesc, sizeof(D3D11_BUFFER_DESC));
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = 0;
		bufDesc.Usage = D3D11_USAGE_DEFAULT;
		bufDesc.ByteWidth = sizeof(Lights);
		return apDevice->CreateBuffer(&bufDesc, nullptr, mpLightBuffer.ReleaseAndGetAddressOf());
	}

	void Scene::PrivUpdateLights(ID3D11DeviceContext1* apContext, const float aDeltaTime)
	{
		mLights.mEyePosition = mpCamera->GetPosition();
		apContext->UpdateSubresource(mpLightBuffer.Get(), 0, nullptr, &mLights, 0, 0);
	}
}
