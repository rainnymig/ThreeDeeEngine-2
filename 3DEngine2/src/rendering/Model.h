#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace tde
{
	class VertexShader;
	class PixelShader;

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

	class Mesh
	{
	public:
		struct MeshVertex
		{
			DirectX::XMFLOAT3 mPosition;
			DirectX::XMFLOAT3 mNormal;
			DirectX::XMFLOAT2 mTexCoord;
		};

		void Draw(
			DirectX::CXMMATRIX aWorldMatrix,
			DirectX::CXMMATRIX aInverseWorldMatrix,
			DirectX::CXMMATRIX aViewProjMatrix,	// world view projection matrix
			ID3D11DeviceContext* apContext,
			std::shared_ptr<VertexShader> apVertexShader,
			std::shared_ptr<PixelShader> apPixelShader) const;

		HRESULT CreateBuffers(ID3D11Device* apDevice);
		void DestroyBuffers();

		std::vector<MeshVertex> mVertices;
		std::vector<uint32_t> mIndices;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpMaterialBuffer;
		size_t mIndexCount;
	};

	class Model : public ConstructorTagHelper
	{
	public:
		struct alignas(16) VertexParams
		{
			DirectX::XMMATRIX mWorldMatrix;
			DirectX::XMMATRIX mInverseWorldMatrix;
			DirectX::XMMATRIX mViewProjMatrix;
		};

		Model(ConstructorTag tag);
		Model(const Model& aOther) = delete;
		Model& operator=(const Model& aOther) = delete;
		~Model();

		void Draw(
			DirectX::CXMMATRIX aWorldMatrix,
			DirectX::CXMMATRIX aViewProjMatrix,	// world view projection matrix
			ID3D11DeviceContext* apContext,
			std::shared_ptr<VertexShader> apVertexShader,
			std::shared_ptr<PixelShader> apPixelShader,
			ID3D11Buffer** appLightBuffer) const;

		static std::shared_ptr<Model> CreateModelFromFile(const char* aPath);

		HRESULT CreateBuffers(ID3D11Device* apDevice);
		void DestroyBuffers();

	private:
		bool PrivLoadModel(const char* aPath);
		void PrivProcessNode(const aiNode* apNode, const aiScene* apScene, const aiMatrix4x4& aParentTransform);
		
		std::vector<Mesh> mMeshes;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpVertexParamsBuffer;
	};
}