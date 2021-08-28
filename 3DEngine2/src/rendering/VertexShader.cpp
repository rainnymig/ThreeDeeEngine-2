#include "pch.h"
#include "rendering/VertexShader.h"

namespace tde
{
	VertexShader::VertexShader(LPCWSTR aVsPath, const D3D11_INPUT_ELEMENT_DESC* apLayout, UINT aLayoutElementCount, ID3D11Device* apDevice)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> pVertexShaderBlob;
		D3DReadFileToBlob(aVsPath, pVertexShaderBlob.ReleaseAndGetAddressOf());
		apDevice->CreateVertexShader(
			pVertexShaderBlob->GetBufferPointer(), 
			pVertexShaderBlob->GetBufferSize(), 
			nullptr, 
			mpVertexShader.ReleaseAndGetAddressOf());

		apDevice->CreateInputLayout(apLayout, aLayoutElementCount,
			pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(),
			mpInputLayout.ReleaseAndGetAddressOf());

		SAFE_RELEASE(pVertexShaderBlob);
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

	void VertexShader::SetInputLayout(ID3D11DeviceContext* apContext) const
	{
		apContext->IASetInputLayout(mpInputLayout.Get());
	}

}