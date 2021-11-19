#include "pch.h"
#include "rendering/CubeWorldRenderer.h"

#include "rendering/Camera.h"
#include "rendering/VertexShader.h"
#include "rendering/PixelShader.h"
#include "rendering/RenderingStateCache.h"

#include <iostream>
#include <fstream>
#include <sstream>

namespace tde
{
	using namespace DirectX;

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
		if (aX >= mSizeX || aY >= mSizeY || aZ >= mSizeZ ||
			aX < 0 || aY < 0 || aZ < 0)
		{
			throw std::runtime_error("failed to access out of bound cube cell of the cube world");
		}

		return mWorld.at((mSizeX * mSizeZ) * aY + mSizeX * aZ + aX);
	}

	CubeWorldRenderer::CubeWorldRenderer(ID3D11Device* apDevice, 
		std::shared_ptr<CubeWorld> apCubeWorld, 
		std::shared_ptr<ICamera> apCamera, 
		ID3D11Buffer** appLightBuffer)
		: mpCubeWorld(apCubeWorld)
		, mpCamera(apCamera)
		, mppLightBuffer(appLightBuffer)
	{
		mpVertexShader = VertexShaderCacheLocator::Get()->Get("BoxVS");
		mpPixelShader = PixelShaderCacheLocator::Get()->Get("BoxPS");

		//	create vertex param constant buffer
		{
			D3D11_BUFFER_DESC bufDesc = { 0 };
			bufDesc.ByteWidth = sizeof(CubeParams);
			bufDesc.Usage = D3D11_USAGE_DEFAULT;
			bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufDesc.CPUAccessFlags = 0;
			bufDesc.MiscFlags = 0;
			apDevice->CreateBuffer(&bufDesc, nullptr, mpVertexParamBuffer.ReleaseAndGetAddressOf());
		}

		//	create material constant buffer
		{
			Material material;
			material.mAmbientColor = DirectX::XMVectorSet(0.75f, 0.9f, 0.9f, 1.0f);
			material.mAmbientCoef = 0.2f;
			material.mDiffuseColor = DirectX::XMVectorSet(0.75f, 0.75f, 0.75f, 1.0f);
			material.mDiffuseCoef = 0.6f;
			material.mSpecularColor = DirectX::XMVectorSet(0.8f, 0.8f, 0.8f, 1.0f);
			material.mSpecularCoef = 0.2f;
			material.mEmissiveColor = DirectX::XMVectorZero();
			material.mEmissiveCoef = 0.0f;
			material.mSpecularPower = 4;
			material.mUseTexture = false;

			D3D11_BUFFER_DESC bufDesc = { 0 };
			bufDesc.ByteWidth = sizeof(Material);
			bufDesc.Usage = D3D11_USAGE_DEFAULT;
			bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufDesc.CPUAccessFlags = 0;
			bufDesc.MiscFlags = 0;
			D3D11_SUBRESOURCE_DATA initData = { 0 };
			initData.pSysMem = &material;
			initData.SysMemPitch = 0;
			initData.SysMemSlicePitch = 0;
			apDevice->CreateBuffer(&bufDesc, &initData, mpMaterialBuffer.ReleaseAndGetAddressOf());
		}
	}

	void CubeWorldRenderer::UpdateBuffer(ID3D11Device* apDevice)
	{
		mpVertexBuffer.Reset();
		
		if (!mpCubeWorld ||
			mpCubeWorld->mSizeX <= 0 || 
			mpCubeWorld->mSizeY <= 0 ||
			mpCubeWorld->mSizeZ <= 0) 
		{
			return;
		}

		std::vector<CubeVertex> vertices;

		float originX = static_cast<float>(mpCubeWorld->mSizeX) / 2.0f;
		float originY = static_cast<float>(mpCubeWorld->mSizeY) / 2.0f;
		float originZ = static_cast<float>(mpCubeWorld->mSizeZ) / 2.0f;

		//	construct the vertices
		for (int y = 0; y < mpCubeWorld->mSizeY; y++)
		{
			for (int z = 0; z < mpCubeWorld->mSizeZ; z++)
			{
				for (int x = 0; x < mpCubeWorld->mSizeX; x++)
				{
					if (mpCubeWorld->At(x, y, z) & HAS_CUBE)
					{
						XMFLOAT3 cubeCenter{
							static_cast<float>(x) - originX + 0.5f,
							static_cast<float>(y) - originY + 0.5f,
							static_cast<float>(z) - originZ + 0.5f };

						//	-x
						if (x == 0 || !(mpCubeWorld->At(x - 1, y, z) & HAS_CUBE))
						{
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_PY_NZ | CubeVertexFacing::NX | CubeTexCoord::TOP_RIGHT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_PY_PZ | CubeVertexFacing::NX | CubeTexCoord::TOP_LEFT});
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_NY_PZ | CubeVertexFacing::NX | CubeTexCoord::BOTTOM_LEFT});

							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_NY_PZ | CubeVertexFacing::NX | CubeTexCoord::BOTTOM_LEFT});
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_NY_NZ | CubeVertexFacing::NX | CubeTexCoord::BOTTOM_RIGHT});
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_PY_NZ | CubeVertexFacing::NX | CubeTexCoord::TOP_RIGHT});
						}
						
						//	-y
						if (y == 0 || !(mpCubeWorld->At(x, y - 1, z) & HAS_CUBE))
						{
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_NY_NZ | CubeVertexFacing::NY | CubeTexCoord::TOP_RIGHT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_NY_NZ | CubeVertexFacing::NY | CubeTexCoord::TOP_LEFT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_NY_PZ | CubeVertexFacing::NY | CubeTexCoord::BOTTOM_LEFT });

							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_NY_PZ | CubeVertexFacing::NY | CubeTexCoord::BOTTOM_LEFT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_NY_PZ | CubeVertexFacing::NY | CubeTexCoord::BOTTOM_RIGHT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_NY_NZ | CubeVertexFacing::NY | CubeTexCoord::TOP_RIGHT });
						}
						
						//	-z
						if (z == 0 || !(mpCubeWorld->At(x, y, z - 1) & HAS_CUBE))
						{
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_PY_NZ | CubeVertexFacing::NZ | CubeTexCoord::TOP_RIGHT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_PY_NZ | CubeVertexFacing::NZ | CubeTexCoord::TOP_LEFT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_NY_NZ | CubeVertexFacing::NZ | CubeTexCoord::BOTTOM_LEFT });
							
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_NY_NZ | CubeVertexFacing::NZ | CubeTexCoord::BOTTOM_LEFT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_NY_NZ | CubeVertexFacing::NZ | CubeTexCoord::BOTTOM_RIGHT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_PY_NZ | CubeVertexFacing::NZ | CubeTexCoord::TOP_RIGHT });
						}
						
						//	+x
						if (x == mpCubeWorld->mSizeX - 1 || !(mpCubeWorld->At(x + 1, y, z) & HAS_CUBE))
						{
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_PY_PZ | CubeVertexFacing::PX | CubeTexCoord::TOP_RIGHT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_PY_NZ | CubeVertexFacing::PX | CubeTexCoord::TOP_LEFT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_NY_NZ | CubeVertexFacing::PX | CubeTexCoord::BOTTOM_LEFT });

							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_NY_NZ | CubeVertexFacing::PX | CubeTexCoord::BOTTOM_LEFT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_NY_PZ | CubeVertexFacing::PX | CubeTexCoord::BOTTOM_RIGHT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_PY_PZ | CubeVertexFacing::PX | CubeTexCoord::TOP_RIGHT });
						}
						
						//	+y
						if (y == mpCubeWorld->mSizeY - 1 || !(mpCubeWorld->At(x, y + 1, z) & HAS_CUBE))
						{
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_PY_PZ | CubeVertexFacing::PY | CubeTexCoord::TOP_RIGHT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_PY_PZ | CubeVertexFacing::PY | CubeTexCoord::TOP_LEFT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_PY_NZ | CubeVertexFacing::PY | CubeTexCoord::BOTTOM_LEFT });

							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_PY_NZ | CubeVertexFacing::PY | CubeTexCoord::BOTTOM_LEFT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_PY_NZ | CubeVertexFacing::PY | CubeTexCoord::BOTTOM_RIGHT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_PY_PZ | CubeVertexFacing::PY | CubeTexCoord::TOP_RIGHT });
						}
						
						//	+z
						if (z == mpCubeWorld->mSizeZ - 1 || !(mpCubeWorld->At(x, y, z + 1) & HAS_CUBE))
						{
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_PY_PZ | CubeVertexFacing::PZ | CubeTexCoord::TOP_RIGHT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_PY_PZ | CubeVertexFacing::PZ | CubeTexCoord::TOP_LEFT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_NY_PZ | CubeVertexFacing::PZ | CubeTexCoord::BOTTOM_LEFT });
							
							vertices.push_back({ cubeCenter, CubeVertexIndex::PX_NY_PZ | CubeVertexFacing::PZ | CubeTexCoord::BOTTOM_LEFT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_NY_PZ | CubeVertexFacing::PZ | CubeTexCoord::BOTTOM_RIGHT });
							vertices.push_back({ cubeCenter, CubeVertexIndex::NX_PY_PZ | CubeVertexFacing::PZ | CubeTexCoord::TOP_RIGHT });
						}
					}
				}
			}
		}
		//vertices.push_back({ {0.0f, 0.0f, 0.0f}, CubeVertexIndex::PX_PY_NZ | CubeVertexFacing::NZ | CubeTexCoord::TOP_RIGHT });
		//vertices.push_back({ {0.0f, 0.0f, 0.0f}, CubeVertexIndex::NX_PY_NZ | CubeVertexFacing::NZ | CubeTexCoord::TOP_LEFT });
		//vertices.push_back({ {0.0f, 0.0f, 0.0f}, CubeVertexIndex::NX_NY_NZ | CubeVertexFacing::NZ | CubeTexCoord::BOTTOM_LEFT });

		//vertices.push_back({ {0.0f, 0.0f, 0.0f}, CubeVertexIndex::NX_NY_NZ | CubeVertexFacing::NZ | CubeTexCoord::BOTTOM_LEFT });
		//vertices.push_back({ {0.0f, 0.0f, 0.0f}, CubeVertexIndex::PX_NY_NZ | CubeVertexFacing::NZ | CubeTexCoord::BOTTOM_RIGHT });
		//vertices.push_back({ {0.0f, 0.0f, 0.0f}, CubeVertexIndex::PX_PY_NZ | CubeVertexFacing::NZ | CubeTexCoord::TOP_RIGHT });
		//mCubeWorldVerticesCount = 6;
		mCubeWorldVerticesCount = vertices.size();

		//	create the vertex buffer
		D3D11_SUBRESOURCE_DATA initialData = { 0 };
		D3D11_BUFFER_DESC bufferDescription = { 0 };

		initialData.pSysMem = &vertices[0];
		initialData.SysMemPitch = 0;
		initialData.SysMemSlicePitch = 0;

		bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDescription.ByteWidth = mCubeWorldVerticesCount * sizeof(CubeVertex);
		bufferDescription.CPUAccessFlags = 0;
		bufferDescription.MiscFlags = 0;
		bufferDescription.Usage = D3D11_USAGE_DEFAULT;

		HRESULT hr = apDevice->CreateBuffer(&bufferDescription, &initialData, mpVertexBuffer.ReleaseAndGetAddressOf());
	}

	void CubeWorldRenderer::SetPosition(DirectX::SimpleMath::Vector4 aCenterPosition)
	{
		mPosition = aCenterPosition;
		mTransformDirty = true;
	}

	void CubeWorldRenderer::SetScale(const float aScale)
	{
		mScale = aScale;
		mTransformDirty = true;
	}

	XMMATRIX CubeWorldRenderer::GetWorldMatrix()
	{
		if (mTransformDirty)
		{
			mWorldMatrix = XMMatrixTranslationFromVector(mPosition) * XMMatrixScaling(mScale, mScale, mScale);
			mTransformDirty = false;
		}
		return mWorldMatrix;
	}
	
	void CubeWorldRenderer::Render(ID3D11DeviceContext* apContext, const float aDeltaTime)
	{
		//	update vertex shader contant buffer which contains matrices
		XMMATRIX viewProj = mpCamera->GetViewMatrix() * mpCamera->GetProjectionMatrix();
		XMMATRIX world = GetWorldMatrix();
		XMMATRIX inversedTransposedWorld = XMMatrixTranspose(XMMatrixInverse(nullptr, world));
		CubeParams vParams{ world, viewProj, inversedTransposedWorld };
		apContext->UpdateSubresource(mpVertexParamBuffer.Get(), 0, nullptr, &vParams, 0, 0);

		//	render
		UINT strides[] = { sizeof(CubeVertex) };
		UINT offsets[] = { 0 };
		//	IA
		mpVertexShader->SetInputLayout(apContext);
		apContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		apContext->IASetVertexBuffers(0, 1, mpVertexBuffer.GetAddressOf(), strides, offsets);
		//	VS
		apContext->VSSetShader(mpVertexShader->GetVertexShader(), nullptr, 0);
		apContext->VSSetConstantBuffers(0, 1, mpVertexParamBuffer.GetAddressOf());
		//	PS
		ID3D11Buffer* pPsConstBufs[2] = { *mppLightBuffer, mpMaterialBuffer.Get() };
		apContext->PSSetShader(mpPixelShader->GetPixelShader(), nullptr, 0);
		apContext->PSSetConstantBuffers(0, 2, pPsConstBufs);
		//	draw
		apContext->Draw(mCubeWorldVerticesCount, 0);
	}
}
