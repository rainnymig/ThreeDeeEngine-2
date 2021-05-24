#pragma once

namespace tde
{
	class IGameObject;
	class ICamera;
	class BaseCamera;

	class Scene
	{
	public:

		void Init(ID3D11Device1* apDevice, HWND aWindowHandle);
		void Update(const float aDeltaTime);
		void Render(ID3D11DeviceContext1* apContext, const float aDeltaTime);
		void Destroy();

	private:
		std::vector<std::shared_ptr<IGameObject>> mGameObjects;
		std::shared_ptr<BaseCamera> mpCamera;
	};
}