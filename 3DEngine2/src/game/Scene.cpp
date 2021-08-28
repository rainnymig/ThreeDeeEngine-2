#include "pch.h"
#include "game/Scene.h"

#include "game/GameObject.h"
#include "rendering/Camera.h"
#include "common/ServiceLocator.h"
#include "common/BaseCache.h"
#include "rendering/VertexShader.h"
#include "rendering/PixelShader.h"

namespace tde
{
	void Scene::Init(ID3D11Device1* apDevice, HWND aWindowHandle)
	{
		//	load shaders
		const D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = sizeof(layout) / sizeof(layout[0]);
		std::shared_ptr<VertexShader> pBasicVS = std::make_shared<VertexShader>(L"shaders/BasicVS.cso", &layout[0], numElements, apDevice);
		if (pBasicVS)
		{
			VertexShaderCacheLocator::Get()->InsertIfNotExists("BasicVS", pBasicVS);
		}

		std::shared_ptr<PixelShader> pPhongPS = std::make_shared<PixelShader>(L"shaders/PhongPS.cso", apDevice);
		if (pPhongPS)
		{
			PixelShaderCacheLocator::Get()->InsertIfNotExists("PhongPS", pPhongPS);
		}

		//	create camera
		mpCamera = std::make_shared<FreeFlightCamera>(aWindowHandle);
		mpCamera->SetPosition({ 0, 3, -10, 1 });
		mpCamera->SetRotation(DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(10), 0, 0));

		//	prepare lights
		mLights.mLights[0].mIsEnabled = true;
		mLights.mLights[0].mType = static_cast<int>(LightType::DIRECTIONAL);
		mLights.mLights[0].mColor = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		mLights.mLights[0].mDirection = DirectX::XMVectorSet(-1.0f, -1.0f, 2.0f, 0.0f);
		mLights.mLights[0].mIntensity = 150.0f;

		mLights.mLights[1].mIsEnabled = false;

		PrivCreateLightBuffer(apDevice);

		//	spawn game objects
		std::shared_ptr<SimpleModelGameObject> go = std::make_shared<SimpleModelGameObject>();
		go->Init("N:\\3DEngine2\\3DEngine2\\x64\\Debug\\Fortnite-Plane.fbx", apDevice, mpCamera, pBasicVS, pPhongPS, mpLightBuffer.GetAddressOf());
		mGameObjects.emplace_back(go);
	}

	void Scene::Update(const float aDeltaTime)
	{
		mpCamera->Update(aDeltaTime);
		for (auto pGameObjects : mGameObjects)
		{
			pGameObjects->Update(aDeltaTime);
		}
	}

	void Scene::Render(ID3D11DeviceContext1* apContext, const float aDeltaTime)
	{
		PrivUpdateLights(apContext, aDeltaTime);

		for (auto pGameObjects : mGameObjects)
		{
			pGameObjects->Render(apContext, aDeltaTime);
		}
	}

	void Scene::Destroy()
	{
		for (auto pGameObjects : mGameObjects)
		{
			pGameObjects->Destroy();
		}
		mGameObjects.clear();
		mpCamera.reset();
		SAFE_RELEASE(mpLightBuffer);
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