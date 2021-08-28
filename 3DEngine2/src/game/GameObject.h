#pragma once

namespace tde
{
	class ICamera;
	class Model;
	class VertexShader;
	class PixelShader;

	class IGameObject
	{
	public:
		virtual void Update(const float deltaTime) = 0;
		virtual void Render(ID3D11DeviceContext1* apContext, const float aDeltaTime) = 0;
		virtual void Destroy() = 0;
	};

	class SimpleModelGameObject : public IGameObject
	{
	public:
		void Init(const char* aModelFilename, ID3D11Device1* apDevice, std::shared_ptr<ICamera> apCamera,
			std::shared_ptr<VertexShader> apVertexShader, std::shared_ptr<PixelShader> apPixelShader, ID3D11Buffer** appLightBuffer);
		virtual void Update(const float aDeltaTime) override;
		virtual void Render(ID3D11DeviceContext1* apContext, const float aDeltaTime) override;
		virtual void Destroy() override;
	private:
		std::shared_ptr<ICamera> mpCamera;
		std::shared_ptr<Model> mpModel;
		DirectX::SimpleMath::Matrix mWorldMatrix;
		std::shared_ptr<VertexShader> mpVertexShader;
		std::shared_ptr<PixelShader> mpPixelShader;
		ID3D11Buffer** mppLightBuffer;
	};
}