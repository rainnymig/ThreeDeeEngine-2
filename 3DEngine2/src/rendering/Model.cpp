#include "pch.h"
#include "rendering/Model.h"
#include "rendering/VertexShader.h"
#include "rendering/PixelShader.h"

namespace tde
{
	Model::Model(ConstructorTag tag)
	{
	}

	Model::~Model()
	{
	}

	void Model::Draw(
		DirectX::CXMMATRIX aWorldMatrix,
		DirectX::CXMMATRIX aViewProjMatrix,	// world view projection matrix
		ID3D11DeviceContext* apContext, 
		std::shared_ptr<VertexShader> apVertexShader, 
		std::shared_ptr<PixelShader> apPixelShader,
		ID3D11Buffer** appLightBuffer) const
	{
		//	update matrices of vertex shader constant buffer
		DirectX::XMMATRIX inversedTransposedWorld = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, aWorldMatrix));
		VertexParams vParams{
			aWorldMatrix, inversedTransposedWorld, aViewProjMatrix
		};
		apContext->UpdateSubresource(mpVertexParamsBuffer.Get(), 0, nullptr, &vParams, 0, 0);

		apVertexShader->SetInputLayout(apContext);
		apContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		apContext->VSSetShader(apVertexShader->GetVertexShader(), nullptr, 0);
		apContext->VSSetConstantBuffers(0, 1, mpVertexParamsBuffer.GetAddressOf());
		apContext->PSSetShader(apPixelShader->GetPixelShader(), nullptr, 0);
		apContext->PSSetConstantBuffers(0, 1, appLightBuffer);

		for (const auto& aMesh : mMeshes)
		{
			aMesh.Draw(aWorldMatrix, inversedTransposedWorld, aViewProjMatrix, apContext, apVertexShader, apPixelShader);
		}
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

		//	create constant buffer for the vertex shader
		D3D11_BUFFER_DESC bufDesc;
		ZeroMemory(&bufDesc, sizeof(D3D11_BUFFER_DESC));
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = 0;
		bufDesc.Usage = D3D11_USAGE_DEFAULT;
		bufDesc.ByteWidth = sizeof(VertexParams);
		hr = apDevice->CreateBuffer(&bufDesc, nullptr, mpVertexParamsBuffer.ReleaseAndGetAddressOf());

		return hr;
	}

	void Model::DestroyBuffers()
	{
		for (auto& mesh : mMeshes)
		{
			mesh.DestroyBuffers();
		}
		SAFE_RELEASE(mpVertexParamsBuffer);
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
			aiProcess_FixInfacingNormals |
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
		aiMatrix4x4 totalTransformForNormal = totalTransform;
		totalTransformForNormal = totalTransformForNormal.Inverse().Transpose();

		for (int i = 0; i < apNode->mNumMeshes; i++)
		{
			Mesh mesh;
			aiMesh* importedMesh = apScene->mMeshes[apNode->mMeshes[i]];
			mesh.mVertices = std::vector<Mesh::MeshVertex>(importedMesh->mNumVertices);
			for (int j = 0; j < importedMesh->mNumVertices; j++)
			{
				auto transformedVertex = importedMesh->mVertices[j];
				auto transformedNormal = importedMesh->mNormals[j];
				transformedVertex *= totalTransform;
				transformedNormal *= totalTransformForNormal;
				mesh.mVertices[j].mPosition.x = transformedVertex.x;
				mesh.mVertices[j].mPosition.y = transformedVertex.y;
				mesh.mVertices[j].mPosition.z = transformedVertex.z;
				mesh.mVertices[j].mNormal.x = transformedNormal.x;
				mesh.mVertices[j].mNormal.y = transformedNormal.y;
				mesh.mVertices[j].mNormal.z = transformedNormal.z;
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

	void Mesh::Draw(
		DirectX::CXMMATRIX aWorldMatrix,
		DirectX::CXMMATRIX aInverseTransposedWorldMatrix,
		DirectX::CXMMATRIX aViewProjMatrix,	// world view projection matrix
		ID3D11DeviceContext* apContext, 
		std::shared_ptr<VertexShader> apVertexShader, 
		std::shared_ptr<PixelShader> apPixelShader) const
	{
		UINT strides[] = { sizeof(MeshVertex) };
		UINT offsets[] = { 0 };
		apContext->IASetVertexBuffers(0, 1, mpVertexBuffer.GetAddressOf(), strides, offsets);
		apContext->IASetIndexBuffer(mpIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		apContext->PSSetConstantBuffers(1, 1, mpMaterialBuffer.GetAddressOf());

		apContext->DrawIndexed(mIndices.size(), 0, 0);
	}

	HRESULT Mesh::CreateBuffers(ID3D11Device* apDevice)
	{
		HRESULT hr;

		D3D11_SUBRESOURCE_DATA initialData = { 0 };
		D3D11_BUFFER_DESC bufferDescription = { 0 };

		//	create vertex buffer
		initialData.pSysMem = &mVertices[0];
		initialData.SysMemPitch = 0;
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

		RETURN_IF_FAILED(hr);

		//	create index buffer
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

		RETURN_IF_FAILED(hr);

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

		//	create constant buffer for material
		bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDescription.ByteWidth = sizeof(Material);
		initialData.pSysMem = &material;
		initialData.SysMemPitch = 0;
		hr = apDevice->CreateBuffer(
			&bufferDescription,
			&initialData,
			mpMaterialBuffer.ReleaseAndGetAddressOf());

		return hr;
	}

	void Mesh::DestroyBuffers()
	{
		SAFE_RELEASE(mpVertexBuffer);
		SAFE_RELEASE(mpIndexBuffer);
		mIndexCount = 0;
	}

}