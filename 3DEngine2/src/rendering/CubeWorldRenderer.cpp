#include "pch.h"
#include "rendering/CubeWorldRenderer.h"

#include <iostream>
#include <fstream>
#include <sstream>

namespace tde
{
	std::shared_ptr<CubeWorld> createCubeWorldFromBinaryFile(LPCSTR aFilename, const size_t aSizeX, const size_t aSizeY, const size_t aSizeZ)
	{
		if (aSizeX > MAX_CUBE_WORLD_SIZE ||
			aSizeY > MAX_CUBE_WORLD_SIZE ||
			aSizeZ > MAX_CUBE_WORLD_SIZE)
		{
			return nullptr;
		}

		std::ifstream cubeFile(aFilename);
		if (!cubeFile.is_open())
		{
			return nullptr;
		}

		std::shared_ptr<CubeWorld> pCubeWorld = std::make_shared<CubeWorld>();
		pCubeWorld->mSizeX = aSizeX;
		pCubeWorld->mSizeY = aSizeY;
		pCubeWorld->mSizeZ = aSizeZ;
		pCubeWorld->mWorld.resize(aSizeX * aSizeY * aSizeZ);

		//	the cube world is stored left to right (x++), back to front (z++), bottom to top (y++)
		cubeFile.read(&(pCubeWorld->mWorld[0]), aSizeX * aSizeY * aSizeZ);

		return pCubeWorld;
	}

	inline CubeCell& CubeWorld::At(const size_t aX, const size_t aY, const size_t aZ)
	{
		if (aX >= mSizeX || aY >= mSizeY || aZ >= mSizeZ)
		{
			throw std::runtime_error("failed to access out of bound cube cell of the cube world");
		}

		return mWorld.at((mSizeX * mSizeZ) * aY + mSizeX * aZ + aX);
	}
}
