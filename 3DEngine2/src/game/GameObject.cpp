#include "pch.h"
#include "game/GameObject.h"
#include "rendering/Camera.h"
#include "rendering/Model.h"

using namespace DirectX;

namespace tde
{
	void SimpleModelGameObject::Init(
		const char* aModelFilename, 
		ID3D11Device1* apDevice, 
		std::shared_ptr<ICamera> apCamera, 
		std::shared_ptr<VertexShader> apVertexShader, 
		std::shared_ptr<PixelShader> apPixelShader, 
		ID3D11Buffer** appLightBuffer)
	{
		mpModel = Model::CreateModelFromFile(aModelFilename);
		mpModel->CreateBuffers(apDevice);
		mWorldMatrix = XMMatrixRotationAxis({ 0, 1.0f, 0, 0 }, XMConvertToRadians(-90.0f)) * XMMatrixScaling(0.01f, 0.01f, 0.01f);
		mpCamera = apCamera;
		mpVertexShader = apVertexShader;
		mpPixelShader = apPixelShader;
		mppLightBuffer = appLightBuffer;
	}

	void SimpleModelGameObject::Update(const float aDeltaTime)
	{
		mWorldMatrix *= XMMatrixRotationAxis({ 0.0f, 1.0f, 0.0f, 0.0f }, XMConvertToRadians(aDeltaTime * 45));
	}

	void SimpleModelGameObject::Render(ID3D11DeviceContext1* apContext, const float aDeltaTime)
	{
		DirectX::XMMATRIX viewProj = mpCamera->GetViewMatrix() * mpCamera->GetProjectionMatrix();
		mpModel->Draw(mWorldMatrix, viewProj, apContext, mpVertexShader, mpPixelShader, mppLightBuffer);
	}

	void SimpleModelGameObject::Destroy()
	{

	}
}