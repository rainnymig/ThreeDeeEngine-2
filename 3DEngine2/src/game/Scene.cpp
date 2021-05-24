#include "pch.h"
#include "game/Scene.h"

#include "game/GameObject.h"
#include "rendering/Camera.h"

namespace tde
{
	void Scene::Init(ID3D11Device1* apDevice, HWND aWindowHandle)
	{
		mpCamera = std::make_shared<FreeFlightCamera>(aWindowHandle);
		mpCamera->SetPosition({ 0, 3, -10, 1 });
		mpCamera->SetRotation(DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(10), 0, 0));

		std::shared_ptr<SimpleModelGameObject> go = std::make_shared<SimpleModelGameObject>();
		go->Init(L"Fortnite-Plane.cmo", apDevice, mpCamera);
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
	}
}