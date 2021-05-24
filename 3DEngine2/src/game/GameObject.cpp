#include "pch.h"
#include "game/GameObject.h"
#include "rendering/Camera.h"

using namespace DirectX;

namespace tde
{
	void SimpleModelGameObject::Init(
		const wchar_t* aModelFilename, 
		ID3D11Device1* apDevice, 
		std::shared_ptr<ICamera> apCamera)
	{
		//mpCommonStates = std::make_unique<CommonStates>(apDevice);
		//mpEffectFactory = std::make_unique<EffectFactory>(apDevice);
		//mpModel = Model::CreateFromCMO(apDevice, aModelFilename, *mpEffectFactory);
		//mpCamera = apCamera;
		//mWorldMatrix = XMMatrixRotationAxis({ 0, 1.0f, 0, 0 }, XMConvertToRadians(-90.0f));
	}

	void SimpleModelGameObject::Update(const float aDeltaTime)
	{
		//mWorldMatrix *= XMMatrixRotationAxis({ 0.0f, 1.0f, 0.0f, 0.0f }, XMConvertToRadians(aDeltaTime * 45));
	}

	void SimpleModelGameObject::Render(ID3D11DeviceContext1* apContext, const float aDeltaTime)
	{
		//mpModel->Draw(apContext, *mpCommonStates, mWorldMatrix, mpCamera->GetViewMatrix(), mpCamera->GetProjectionMatrix());
	}

	void SimpleModelGameObject::Destroy()
	{
		//mpCommonStates.reset();
		//mpEffectFactory.reset();
		//mpModel.reset();
	}
}