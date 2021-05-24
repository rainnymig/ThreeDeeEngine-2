#include "pch.h"
#include "rendering/PixelShader.h"

namespace tde
{
	PixelShader::PixelShader(LPCWSTR aPsPath, ID3D11Device* apDevice)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> pPixelShaderBlob;
		D3DReadFileToBlob(aPsPath, pPixelShaderBlob.ReleaseAndGetAddressOf());
		apDevice->CreatePixelShader(
			pPixelShaderBlob->GetBufferPointer(),
			pPixelShaderBlob->GetBufferSize(),
			nullptr,
			mpPixelShader.ReleaseAndGetAddressOf());
		pPixelShaderBlob.Reset();
	}

	PixelShader::PixelShader(PixelShader&& aOther) noexcept
	{
		mpPixelShader = std::move(aOther.mpPixelShader);
		aOther.mpPixelShader = nullptr;
	}

	PixelShader& PixelShader::operator=(PixelShader&& aOther) noexcept
	{
		if (this == &aOther)
		{
			return *this;
		}
		mpPixelShader.Reset();
		mpPixelShader = std::move(aOther.mpPixelShader);
		aOther.mpPixelShader = nullptr;
		return *this;
	}

	PixelShader::~PixelShader()
	{
		mpPixelShader.Reset();
	}

	ID3D11PixelShader* PixelShader::GetPixelShader() const
	{
		return mpPixelShader.Get();
	}
}