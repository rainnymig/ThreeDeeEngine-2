#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace tde
{
	class Mesh
	{
	public:
		struct MeshVertex
		{
			DirectX::XMFLOAT3 mPosition;
			DirectX::XMFLOAT3 mNormal;
			DirectX::XMFLOAT2 mTexCoord;
		};

		HRESULT CreateBuffers(ID3D11Device* apDevice);
		void DestroyBuffers();

		std::vector<MeshVertex> mVertices;
		std::vector<uint32_t> mIndices;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mpIndexBuffer;
		size_t mIndexCount;
	};

	class Model : public ConstructorTagHelper
	{
	public:
		Model(ConstructorTag tag);
		Model(const Model& aOther) = delete;
		Model& operator=(const Model& aOther) = delete;
		~Model();

		static std::shared_ptr<Model> CreateModelFromFile(const char* aPath);

		HRESULT CreateBuffers(ID3D11Device* apDevice);
		void DestroyBuffers();

	private:
		bool PrivLoadModel(const char* aPath);
		void PrivProcessNode(const aiNode* apNode, const aiScene* apScene, const aiMatrix4x4& aParentTransform);
		
		std::vector<Mesh> mMeshes;
	};
}