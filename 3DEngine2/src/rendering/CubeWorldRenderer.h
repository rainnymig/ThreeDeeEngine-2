#pragma once

namespace tde
{
	using CubeCell = char;

	constexpr static size_t MAX_CUBE_WORLD_SIZE = 100;
	constexpr static CubeCell HAS_CUBE = 0X01;

	struct CubeWorld {
		inline CubeCell& At(const size_t aX, const size_t aY, const size_t aZ);

		std::vector<CubeCell> mWorld;

		//	left - right +
		size_t mSizeX;
		//	bottom - top +
		size_t mSizeY;
		//	back - front +
		size_t mSizeZ;
	};

	std::shared_ptr<CubeWorld> createCubeWorldFromBinaryFile(LPCSTR aFilename, const size_t aSizeX, const size_t aSizeY, const size_t aSizeZ);

	class CubeWorldRenderer
	{
	public:
		CubeWorldRenderer(std::shared_ptr<CubeWorld> apCubeWorld);
		~CubeWorldRenderer();
		
		void Render(ID3D11DeviceContext* apContext, const float aDeltaTime);
	private:
		std::shared_ptr<CubeWorld> mpCubeWorld;
		DirectX::SimpleMath::Vector4 mPosition{ 0.0f, 0.0f, 0.0f, 1.0f };	//	the center point of the cube world
		float mScale = 1.0f;	//	the size of one cube
	};
}
