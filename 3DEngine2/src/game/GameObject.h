#pragma once

namespace tde
{
	class ICamera;

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
		void Init(const wchar_t* aModelFilename, ID3D11Device1* apDevice, std::shared_ptr<ICamera> apCamera);
		virtual void Update(const float aDeltaTime) override;
		virtual void Render(ID3D11DeviceContext1* apContext, const float aDeltaTime) override;
		virtual void Destroy() override;
	private:
		//std::unique_ptr<DirectX::CommonStates> mpCommonStates;
		//std::unique_ptr<DirectX::IEffectFactory> mpEffectFactory;
		//std::unique_ptr<DirectX::Model> mpModel;
		std::shared_ptr<ICamera> mpCamera;
		DirectX::SimpleMath::Matrix mWorldMatrix;
	};
}