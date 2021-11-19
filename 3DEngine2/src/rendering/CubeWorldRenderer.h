#pragma once

namespace tde
{
	class ICamera;
	class VertexShader;
	class PixelShader;

	using CubeCell = char;

	constexpr static size_t MAX_CUBE_WORLD_SIZE = 100;
	constexpr static CubeCell HAS_CUBE = 0X01;

	enum CubeVertexIndex
	{
		NX_NY_NZ = 0x0,
		NX_NY_PZ = 0x1,
		NX_PY_NZ = 0x2,
		NX_PY_PZ = 0x3,
		PX_NY_NZ = 0x4,
		PX_NY_PZ = 0x5,
		PX_PY_NZ = 0x6,
		PX_PY_PZ = 0x7,
	};

	enum CubeVertexFacing
	{
		NX = 0x0 << 3,
		NY = 0x1 << 3,
		NZ = 0x2 << 3,
		PX = 0x3 << 3,
		PY = 0x4 << 3,
		PZ = 0x5 << 3,
	};

	enum CubeTexCoord
	{
		TOP_LEFT		= 0x0 << 6,
		TOP_RIGHT		= 0x1 << 6,
		BOTTOM_LEFT		= 0x2 << 6,
		BOTTOM_RIGHT	= 0x3 << 6,
	};

	struct CubeWorld {
		inline CubeCell& At(const size_t aX, const size_t aY, const size_t aZ);

		std::vector<CubeCell> mWorld;

		//	left - right +
		size_t mSizeX = 8;
		//	bottom - top +
		size_t mSizeY = 8;
		//	back - front +
		size_t mSizeZ = 8;
	};

	std::shared_ptr<CubeWorld> createCubeWorldFromBinaryFile(LPCSTR aFilename, const size_t aSizeX, const size_t aSizeY, const size_t aSizeZ);

	class CubeWorldRenderer
	{
	public:

		struct alignas(16) Material
		{
			DirectX::XMVECTOR mAmbientColor;
			DirectX::XMVECTOR mDiffuseColor;
			DirectX::XMVECTOR mSpecularColor;
			DirectX::XMVECTOR mEmissiveColor;
			float mAmbientCoef;
			float mDiffuseCoef;
			float mSpecularCoef;
			float mEmissiveCoef;
			int mSpecularPower;
			int mUseTexture;
			int mPadding[2];
		};

		struct CubeVertex 
		{
			//	the center position of the cube
			DirectX::XMFLOAT3 mCenterPosition;
			
			//	from LSB
			//	-----------
			//	3 bits
			//	0b000....000***
			//	0: -x -y -z
			//	1: -x -y +z
			//	2: -x +y -z
			//	3: -x +y +z
			//	4: +x -y -z
			//	5: +x -y +z
			//	6: +x +y -z
			//	7: +x +y +z
			//	-----------
			//	3 bits
			//	0b000....000***000
			//	0: facing -x
			//	1: facing -y
			//	2: facing -z
			//	3: facing +x
			//	4: facing +y
			//	5: facing +z
			//	-----------
			//	2 bits
			//	0b000....0**000000
			//	0: top left		(0,0)
			//	1: top right	(0,1)
			//	2: bottom left	(1,0)
			//	3: bottom right (1,1)
			UINT32 mVertex;
		};
		
		struct alignas(16) CubeParams {
			DirectX::XMMATRIX mWorldMatrix;
			DirectX::XMMATRIX mInversedTransposedWorldMatrix;
			DirectX::XMMATRIX mViewProjectionMatrix;
			//DirectX::XMVECTOR mWorldCenterAndScale;
		};

		CubeWorldRenderer(ID3D11Device* apDevice, 
			std::shared_ptr<CubeWorld> apCubeWorld, 
			std::shared_ptr<ICamera> apCamera,
			ID3D11Buffer** appLightBuffer);

		void UpdateBuffer(ID3D11Device* apDevice);
		void SetPosition(DirectX::SimpleMath::Vector4 centerPosition);
		void SetScale(const float aScale);

		void Render(ID3D11DeviceContext* apContext, const float aDeltaTime);
	private:

		DirectX::XMMATRIX GetWorldMatrix();

		std::shared_ptr<ICamera> mpCamera;
		std::shared_ptr<CubeWorld> mpCubeWorld;
		size_t mCubeWorldVerticesCount = 0;
		
		std::shared_ptr<VertexShader> mpVertexShader;
		std::shared_ptr<PixelShader> mpPixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpVertexParamBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpMaterialBuffer;
		ID3D11Buffer** mppLightBuffer;
		
		DirectX::SimpleMath::Matrix mWorldMatrix;
		DirectX::SimpleMath::Vector4 mPosition{ 0.0f, 0.0f, 0.0f, 1.0f };	//	the center point of the cube world
		float mScale = 1.0f;	//	the size of one cube
		bool mTransformDirty = true;
	};
}
