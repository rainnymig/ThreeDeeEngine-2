#include "pch.h"
#include "rendering/VertexShader.h"

namespace tde
{
	VertexShader::VertexShader(LPCWSTR aVsPath, ID3D11Device* apDevice)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> pVertexShaderBlob;
		D3DReadFileToBlob(aVsPath, pVertexShaderBlob.ReleaseAndGetAddressOf());
		apDevice->CreateVertexShader(
			pVertexShaderBlob->GetBufferPointer(), 
			pVertexShaderBlob->GetBufferSize(), 
			nullptr, 
			mpVertexShader.ReleaseAndGetAddressOf());
		pVertexShaderBlob.Reset();
	}

	VertexShader::VertexShader(VertexShader&& aOther) noexcept
	{
		mpVertexShader = std::move(aOther.mpVertexShader);
		aOther.mpVertexShader = nullptr;
	}

	VertexShader& VertexShader::operator=(VertexShader&& aOther) noexcept
	{
		if (this == &aOther)
		{
			return *this;
		}
		mpVertexShader.Reset();
		mpVertexShader = std::move(aOther.mpVertexShader);
		aOther.mpVertexShader = nullptr;
		return *this;
	}

	VertexShader::~VertexShader()
	{
		mpVertexShader.Reset();
	}

	ID3D11VertexShader* VertexShader::GetVertexShader() const
	{
		return mpVertexShader.Get();
	}
}