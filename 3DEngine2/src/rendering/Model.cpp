#include "pch.h"
#include "rendering/Model.h"

namespace tde
{
	Model::Model(ConstructorTag tag)
	{
	}

	Model::~Model()
	{
	}

	std::shared_ptr<Model> Model::CreateModelFromFile(const char* aPath)
	{
		std::shared_ptr<Model> pModel = std::make_shared<Model>(ConstructorTag());
		bool loadModelResult = pModel->PrivLoadModel(aPath);
		if (loadModelResult)
		{
			return pModel;
		}
		else
		{
			return nullptr;
		}
	}

	HRESULT Model::CreateBuffers(ID3D11Device* apDevice)
	{
		HRESULT hr = S_OK;
		if (mMeshes.empty())
		{
			return S_FALSE;
		}
		for (auto& mesh : mMeshes)
		{
			if (!SUCCEEDED(hr = mesh.CreateBuffers(apDevice)))
			{
				break;
			}
		}
		if (!SUCCEEDED(hr))
		{
			DestroyBuffers();
		}
		return hr;
	}

	void Model::DestroyBuffers()
	{
		for (auto& mesh : mMeshes)
		{
			mesh.DestroyBuffers();
		}
	}

	bool Model::PrivLoadModel(const char* aPath)
	{
		Assimp::Importer importer;
		const aiScene* pScene = importer.ReadFile(
			aPath, 
			aiProcess_Triangulate | 
			aiProcess_JoinIdenticalVertices | 
			aiProcess_MakeLeftHanded |
			aiProcess_GenNormals |
			aiProcess_PreTransformVertices |
			aiProcess_GenUVCoords | 
			aiProcess_FlipUVs);

		if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
		{
			return false;
		}

		aiMatrix4x4 rootTransform(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
		PrivProcessNode(pScene->mRootNode, pScene, rootTransform);

		return true;
	}

	void Model::PrivProcessNode(const aiNode* apNode, const aiScene* apScene, const aiMatrix4x4& aParentTransform)
	{
		if (!apNode || !apScene)
		{
			return;
		}

		aiMatrix4x4 totalTransform = aParentTransform * apNode->mTransformation;

		for (int i = 0; i < apNode->mNumMeshes; i++)
		{
			Mesh mesh;
			aiMesh* importedMesh = apScene->mMeshes[apNode->mMeshes[i]];
			mesh.mVertices = std::vector<Mesh::MeshVertex>(importedMesh->mNumVertices);
			for (int j = 0; j < importedMesh->mNumVertices; j++)
			{
				auto transformedVertex = importedMesh->mVertices[j];
				transformedVertex *= totalTransform;
				mesh.mVertices[j].mPosition.x = transformedVertex.x;
				mesh.mVertices[j].mPosition.y = transformedVertex.y;
				mesh.mVertices[j].mPosition.z = transformedVertex.z;
				mesh.mVertices[j].mNormal.x = importedMesh->mNormals[j].x;
				mesh.mVertices[j].mNormal.y = importedMesh->mNormals[j].y;
				mesh.mVertices[j].mNormal.z = importedMesh->mNormals[j].z;
				if (importedMesh->mTextureCoords[0])
				{
					mesh.mVertices[j].mTexCoord.x = importedMesh->mTextureCoords[0][j].x;
					mesh.mVertices[j].mTexCoord.y = importedMesh->mTextureCoords[0][j].y;
				}
			}
			for (int j = 0; j < importedMesh->mNumFaces; j++)
			{
				aiFace& face = importedMesh->mFaces[j];
				for (int k = 0; k < face.mNumIndices; k++)
				{
					mesh.mIndices.emplace_back(face.mIndices[k]);
				}
			}
			mMeshes.emplace_back(mesh);
		}

		for (int i = 0; i < apNode->mNumChildren; i++)
		{
			PrivProcessNode(apNode->mChildren[i], apScene, totalTransform);
		}
	}

	HRESULT Mesh::CreateBuffers(ID3D11Device* apDevice)
	{
		HRESULT hr;

		D3D11_SUBRESOURCE_DATA initialData = { 0 };
		D3D11_BUFFER_DESC bufferDescription = { 0 };

		initialData.pSysMem = &mVertices[0];
		initialData.SysMemPitch = sizeof(MeshVertex);
		initialData.SysMemSlicePitch = 0;

		bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDescription.ByteWidth = mVertices.size() * sizeof(MeshVertex);
		bufferDescription.CPUAccessFlags = 0;
		bufferDescription.MiscFlags = 0;
		bufferDescription.Usage = D3D11_USAGE_DEFAULT;

		hr = apDevice->CreateBuffer(
			&bufferDescription, 
			&initialData, 
			mpVertexBuffer.ReleaseAndGetAddressOf());

		mIndexCount = mIndices.size();

		ZeroMemory(&bufferDescription, sizeof(bufferDescription));
		bufferDescription.Usage = D3D11_USAGE_DEFAULT;
		bufferDescription.ByteWidth = sizeof(uint32_t) * mIndexCount;
		bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDescription.CPUAccessFlags = 0;
		bufferDescription.MiscFlags = 0;

		ZeroMemory(&initialData, sizeof(initialData));
		initialData.pSysMem = &mIndices[0];
		initialData.SysMemPitch = sizeof(uint32_t);
		initialData.SysMemSlicePitch = 0;

		hr = apDevice->CreateBuffer(
			&bufferDescription, 
			&initialData, 
			mpIndexBuffer.ReleaseAndGetAddressOf());

		return hr;
	}

	void Mesh::DestroyBuffers()
	{
		SAFE_RELEASE(mpVertexBuffer);
		SAFE_RELEASE(mpIndexBuffer);
		mIndexCount = 0;
	}

}